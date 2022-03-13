#pragma once
#ifndef FG_INC_PROFILING
#define FG_INC_PROFILING

#include <stack>
#include <unordered_map>

#include <BuildConfig.hpp>
#include <util/Util.hpp>

namespace profile
{
    inline extern const int MAX_PROFILE_SAMPLES = 16;

    struct Sample
    {
        /// Whether this data is valid
        bool isValid;
        /// Number of times profile begin called
        unsigned int numInstances;
        /// Number of times opened w/o profile end
        unsigned int numOpen;
        /// Name of sample
        std::string name;
        /// The current open profile start time
        float startTime;
        /// All samples this frame added together
        float accumulator;
        /// Time taken by all children
        float childrenSampleTime;
        /// Number of profile parents
        unsigned int numParents;
        Sample() : isValid(false), numInstances(0),
                   numOpen(0), name("\0"), startTime(-1.0f), accumulator(0.0f),
                   childrenSampleTime(-1.0f), numParents(0) {}
    }; //# struct Sample

    struct SampleHistory
    {
        /// Whether the data is valid
        bool isValid;
        /// Name of sample
        std::string name;
        /// Average time per frame (percentage)
        float average;
        /// Minimum time per frame %
        float minimum;
        /// Maximum time per frame %
        float maximum;
        SampleHistory() : isValid(false), name("\0"), average(0.0f),
                          minimum(0.0f), maximum(0.0f) {}
    }; //# struct SampleHistory

    class Profiling
    {
    protected:
        using ProfileStack = std::stack<Sample *>;
        using HashKey = std::string;
        using ProfileMap = std::unordered_map<HashKey, Sample *>;
        using HistoryMap = std::unordered_map<HashKey, SampleHistory *>;

        using ProfileMapPair = std::pair<std::string, Sample *>;
        using HistoryMapPair = std::pair<std::string, SampleHistory *>;
        using ProfileVec = std::vector<Sample *>;

    private:
        /// Stack holding currently open samples (active) in order
        ProfileStack m_profileStack;
        /// Stack holding all profiles in order in which they were added
        ProfileVec m_orderVec;
        /// Map for binding string id (name) to profile info structure
        ProfileMap m_sampleMap;
        /// Map holding history records (for average values)
        HistoryMap m_sampleHistory;
        /// When the profiling started (frame)
        float m_startProfile;
        /// When the profiling ended
        float m_endProfile;

    public:
        Profiling();
        virtual ~Profiling();

        void initialize(void);
        void clear(void);

        bool begin(const std::string &name);
        bool begin(const char *name);

        bool end(const std::string &name);
        bool end(const char *name);

        void updateHistory(void);
        void dumpToDefaultFile(void);

        bool storeProfileHistory(const std::string &name, float percent);
        bool getProfileHistory(const std::string &name, float *average,
                               float *minimum, float *maximum);
    }; //# class Profiling
} //> namespace profile

namespace profile
{
#if defined(FG_DEBUG)
    extern Profiling *g_debugProfiling;
#endif
} //> namespace profile

#endif //> FG_INC_PROFILING
