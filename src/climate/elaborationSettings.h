#ifndef ELABORATIONSETTINGS_H
#define ELABORATIONSETTINGS_H

    #define DEF_VALUE_ANOMALY_PTS_MAX_DISTANCE 3000
    #define DEF_VALUE_ANOMALY_PTS_MAX_DELTA_Z 50
    #define DEF_VALUE_GRID_MIN_COVERAGE 0


    class Crit3DElaborationSettings
    {
    public:

        Crit3DElaborationSettings()
        {
            _anomalyPtsMaxDistance = DEF_VALUE_ANOMALY_PTS_MAX_DISTANCE;
            _anomalyPtsMaxDeltaZ = DEF_VALUE_ANOMALY_PTS_MAX_DELTA_Z;
            _gridMinCoverage = DEF_VALUE_GRID_MIN_COVERAGE;
            _mergeJointStations = true;
        }

        double getAnomalyPtsMaxDistance() const {
            return _anomalyPtsMaxDistance;
        }

        void setAnomalyPtsMaxDistance(double value) {
            _anomalyPtsMaxDistance = value;
        }

        double getAnomalyPtsMaxDeltaZ() const {
            return _anomalyPtsMaxDeltaZ;
        }

        void setAnomalyPtsMaxDeltaZ(double value) {
            _anomalyPtsMaxDeltaZ = value;
        }

        double getGridMinCoverage() const {
            return _gridMinCoverage;
        }

        void setGridMinCoverage(double value) {
            _gridMinCoverage = value;
        }

        bool getMergeJointStations() const {
            return _mergeJointStations;
        }

        void setMergeJointStations(bool value) {
            _mergeJointStations = value;
        }

    private:
        double _anomalyPtsMaxDistance;
        double _anomalyPtsMaxDeltaZ;
        double _gridMinCoverage;
        bool _mergeJointStations;

    };

#endif // ELABORATIONSETTINGS_H
