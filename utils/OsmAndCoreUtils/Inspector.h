#ifndef _OSMAND_CORE_UTILS_INSPECTOR_H_
#define _OSMAND_CORE_UTILS_INSPECTOR_H_

#include <OsmAndCore/QtExtensions.h>
#include <QString>
#include <QStringList>

#include <OsmAndCoreUtils.h>
#include <OsmAndCore/CommonTypes.h>

namespace OsmAnd
{
    namespace Inspector
    {
        struct OSMAND_CORE_UTILS_API Configuration
        {
            Configuration();
            Configuration(const QString& fileName);

            QString fileName;
            bool verboseAddress;
            bool verboseStreetGroups;
            bool verboseStreets;
            bool verboseBuildings;
            bool verboseIntersections;
            bool verboseMap;
            bool verboseMapObjects;
            bool verbosePoi;
            bool verboseAmenities;
            bool verboseTrasport;
            AreaD bbox;
            ZoomLevel zoom;
        };
        OSMAND_CORE_UTILS_API bool OSMAND_CORE_UTILS_CALL parseCommandLineArguments(const QStringList& cmdLineArgs, Configuration& cfg, QString& error);
        OSMAND_CORE_UTILS_API void OSMAND_CORE_UTILS_CALL dumpToStdOut(const Configuration& cfg);
        OSMAND_CORE_UTILS_API QString OSMAND_CORE_UTILS_CALL dumpToString(const Configuration& cfg);
    }
}

#endif // !defined(_OSMAND_CORE_UTILS_INSPECTOR_H_)
