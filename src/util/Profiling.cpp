#include <util/Profiling.hpp>
#include <util/Timesys.hpp>
#include <util/RegularFile.hpp>

#if defined(FG_DEBUG)
profile::Profiling *profile::g_debugProfiling = NULL;
#endif

profile::Profiling::Profiling() : m_startProfile(-1.0f), m_endProfile(-1.0f)
{
}
//>---------------------------------------------------------------------------------------

profile::Profiling::~Profiling()
{
    clear();
}
//>---------------------------------------------------------------------------------------

void profile::Profiling::initialize(void)
{
    m_startProfile = timesys::exact();
}
//>---------------------------------------------------------------------------------------

void profile::Profiling::clear(void)
{
    while (!m_profileStack.empty())
        m_profileStack.pop();
    m_sampleMap.clear();
    m_sampleHistory.clear();
    for (auto &it : m_orderVec)
        delete it;
    m_orderVec.clear();
    m_startProfile = 0.0f;
    m_endProfile = 0.0f;
}
//>---------------------------------------------------------------------------------------

bool profile::Profiling::begin(const std::string &name)
{
    if (name.empty())
        return false;
    ProfileMapPair query_pair;
    query_pair.first = name;
    query_pair.second = NULL;
    auto result = m_sampleMap.insert(query_pair);
    auto &it = result.first;
    Sample *sample = it->second;
    if (result.second == false && sample && sample->isValid)
    {
        //  Existed
        if (sample->numOpen)
        {
            // max 1 open at once
            return false;
        }
        sample->numOpen++;
        sample->numInstances++;
        sample->startTime = timesys::exact();
    }
    else
    {
        // New insertion
        if (m_sampleMap.size() > MAX_PROFILE_SAMPLES)
        {
            if (result.second)
            {
                // delete query_pair.second;
                // query_pair.second = NULL;
                // it->second = NULL; // ?
            }
            m_sampleMap.erase(it);
            return false;
        }
        if (!sample)
            it->second = new Sample();
        sample = it->second;
        sample->isValid = true;
        sample->numOpen = 1;
        sample->numInstances = 1;
        sample->accumulator = 0.0f;
        sample->startTime = timesys::exact();
        sample->childrenSampleTime = 0.0f;
        sample->name = name;
        if (result.second)
            m_orderVec.push_back(sample);
    }
    if (sample)
        m_profileStack.push(sample);
    return true;
}
//>---------------------------------------------------------------------------------------

bool profile::Profiling::begin(const char *name)
{
    if (!name)
        return false;
    return begin(std::string(name));
}
//>---------------------------------------------------------------------------------------

bool profile::Profiling::end(const std::string &name)
{
    if (name.empty())
        return false;

    auto it = m_sampleMap.find(name);
    if (it == m_sampleMap.end())
        return false;
    Sample *sample = it->second;
    if (!sample)
        return false;
    if (!sample->isValid || !sample->numOpen)
        return false;
    sample->numOpen--;
    float endTime = timesys::exact();
    m_profileStack.pop();
    sample->numParents = (int)m_profileStack.size();
    if (sample->numParents)
    {
        Sample *parent = m_profileStack.top();
        if (parent->isValid)
            parent->childrenSampleTime += endTime - sample->startTime;
    }
    sample->accumulator += endTime - sample->startTime;
    return true;
}
//>---------------------------------------------------------------------------------------

bool profile::Profiling::end(const char *name)
{
    if (!name)
        return false;
    return end(std::string(name));
}
//>---------------------------------------------------------------------------------------

void profile::Profiling::updateHistory(void)
{
    m_endProfile = timesys::exact();
    ProfileVec::iterator begin = m_orderVec.begin(), end = m_orderVec.end(), it;
    for (it = begin; it != end; it++)
    {
        Sample *sample = (*it);
        if (!sample->isValid)
            continue;
        sample->isValid = false;
        float sampleTime, percentTime, aveTime, minTime, maxTime;
        sampleTime = sample->accumulator - sample->childrenSampleTime;
        percentTime = (sampleTime / (m_endProfile - m_startProfile)) * 100.0f;
        aveTime = minTime = maxTime = percentTime;
        storeProfileHistory(sample->name, percentTime);
    }
    m_startProfile = timesys::exact();
}
//>---------------------------------------------------------------------------------------

void profile::Profiling::dumpToDefaultFile(void)
{
    ProfileVec::iterator begin = m_orderVec.begin(), end = m_orderVec.end(), it;
    util::RegularFile file;
    file.setMode(util::RegularFile::Mode::WRITE);
    file.open("defaultProfile.log");
    auto timestamp = timesys::seconds();
    struct tm ti;
    localtime_s(&ti, &timestamp);
    file.print("\n%02d/%02d/%02d %02d:%02d:%02d: Profiles Dump\n",
               ti.tm_mday,
               ti.tm_mon + 1,
               ti.tm_year - 100,
               ti.tm_hour,
               ti.tm_min,
               ti.tm_sec);
    file.print("  Ave :   Min :   Max :   # : Profile Name\n");
    file.print("--------------------------------------------\n");
    for (it = begin; it != end; it++)
    {
        Sample *sample = (*it);
        // if(!sample->isValid)
        //     continue;
        unsigned int indent = 0;
        float aveTime, minTime, maxTime;
        char line[256], name[256], indentedName[256];
        char ave[16], min[16], max[16], num[16];
        this->getProfileHistory(sample->name, &aveTime, &minTime, &maxTime);
        // Format the data
        sprintf(ave, "%3.1f", aveTime);
        sprintf(min, "%3.1f", minTime);
        sprintf(max, "%3.1f", maxTime);
        sprintf(num, "%3d", sample->numInstances);
        strcpy(indentedName, sample->name.c_str());
        for (indent = 0; indent < sample->numParents; indent++)
        {
            sprintf(name, "   %s", indentedName);
            strcpy(indentedName, name);
        }
        sprintf(line, "%5s : %5s : %5s : %3s : %s\n", ave, min, max, num, indentedName);
        file.print(line);
    }
    file.close();
}
//>---------------------------------------------------------------------------------------

bool profile::Profiling::storeProfileHistory(const std::string &name, float percent)
{
    if (name.empty())
        return false;
    float oldRatio;
    float newRatio = 0.8f * timesys::elapsed();
    if (newRatio > 1.0f)
        newRatio = 1.0f;
    oldRatio = 1.0f - newRatio;
    std::pair<std::string, SampleHistory *> query_pair;
    query_pair.first = name;
    query_pair.second = NULL;
    auto result = m_sampleHistory.insert(query_pair);
    auto it = result.first;
    SampleHistory *sample = it->second;
    // if(!sample)
    //     return false;
    if (result.second == false && sample)
    {
        // Sample existed #FIXME -- too much allocs
        // delete query_pair.second;
        // query_pair.second = NULL;
        // sample = it->second;
        // Existed
        sample->average = sample->average * oldRatio + (percent * newRatio);
        if (percent < sample->minimum)
            sample->minimum = percent;
        else
            sample->minimum = sample->minimum * oldRatio + (percent * newRatio);
        if (sample->minimum < 0.0f)
            sample->minimum = 0.0f;
        if (percent > sample->maximum)
            sample->maximum = percent;
        else
            sample->maximum = sample->maximum * oldRatio + (percent * newRatio);
    }
    else
    {
        // New insertion
        if (m_sampleHistory.size() > MAX_PROFILE_SAMPLES)
        {
            // delete it->second;
            // it->second = NULL;
            m_sampleHistory.erase(it);
            return false;
        }
        if (!sample)
            it->second = new SampleHistory();
        sample = it->second;
        sample->isValid = true;
        sample->name = name;
        sample->average = sample->minimum = sample->maximum = percent;
    }
    return true;
}
//>---------------------------------------------------------------------------------------

bool profile::Profiling::getProfileHistory(const std::string &name, float *average,
                                           float *minimum, float *maximum)
{
    if (name.empty())
        return false;
    auto it = m_sampleHistory.find(name);
    if (it == m_sampleHistory.end())
        return false;
    SampleHistory *entry = it->second;
    if (!entry)
        return false;
    if (average)
        *average = entry->average;
    if (minimum)
        *minimum = entry->minimum;
    if (maximum)
        *maximum = entry->maximum;
    return true;
}
//>---------------------------------------------------------------------------------------
