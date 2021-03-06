#ifndef _OSMAND_CORE_OBF_MAP_SECTION_READER_H_
#define _OSMAND_CORE_OBF_MAP_SECTION_READER_H_

#include <OsmAndCore/stdlib_common.h>
#include <functional>

#include <OsmAndCore/QtExtensions.h>
#include <QList>

#include <OsmAndCore.h>
#include <OsmAndCore/CommonTypes.h>
#include <OsmAndCore/Data/DataTypes.h>
#include <OsmAndCore/Map/MapTypes.h>

namespace OsmAnd {

    class ObfReader;
    class ObfMapSectionInfo;
    namespace Model {
        class MapObject;
    } // namespace Model
    class IQueryController;
    namespace ObfMapSectionReader_Metrics {
        struct Metric_loadMapObjects;
    } // namespace ObfMapSectionReader_Metrics

    class OSMAND_CORE_API ObfMapSectionReader
    {
    private:
        ObfMapSectionReader();
        ~ObfMapSectionReader();
    protected:
    public:
        static void loadMapObjects(const std::shared_ptr<ObfReader>& reader, const std::shared_ptr<const ObfMapSectionInfo>& section,
            ZoomLevel zoom, const AreaI* bbox31 = nullptr,
            QList< std::shared_ptr<const OsmAnd::Model::MapObject> >* resultOut = nullptr, MapFoundationType* foundationOut = nullptr,
            const FilterMapObjectsByIdSignature filterById = nullptr,
            std::function<bool (const std::shared_ptr<const OsmAnd::Model::MapObject>&)> visitor = nullptr,
            const IQueryController* const controller = nullptr,
            ObfMapSectionReader_Metrics::Metric_loadMapObjects* const metric = nullptr);
    };

} // namespace OsmAnd

#endif // !defined(_OSMAND_CORE_OBF_MAP_SECTION_READER_H_)
