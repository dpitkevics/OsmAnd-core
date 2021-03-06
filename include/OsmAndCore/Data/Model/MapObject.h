#ifndef _OSMAND_CORE_MAP_OBJECT_H_
#define _OSMAND_CORE_MAP_OBJECT_H_

#include <OsmAndCore/stdlib_common.h>

#include <OsmAndCore/QtExtensions.h>
#include <QList>
#include <QMap>
#include <QVector>
#include <QString>
#include <QHash>

#include <OsmAndCore.h>
#include <OsmAndCore/CommonTypes.h>
#include <OsmAndCore/Map/MapTypes.h>

namespace OsmAnd
{
    class ObfMapSectionInfo;
    class ObfMapSectionLevel;
    class ObfMapSectionReader_P;
    class Rasterizer_P;

    namespace Model
    {
        class OSMAND_CORE_API MapObject
        {
            Q_DISABLE_COPY(MapObject);
        private:
        protected:
            MapObject(const std::shared_ptr<const ObfMapSectionInfo>& section, const std::shared_ptr<const ObfMapSectionLevel>& level);

            uint64_t _id;
            MapFoundationType _foundation;
            bool _isArea;
            QVector< PointI > _points31;
            QList< QVector< PointI > > _innerPolygonsPoints31;
            QVector< uint32_t > _typesRuleIds;
            QVector< uint32_t > _extraTypesRuleIds;
            QMap< uint32_t, QString > _names;
            AreaI _bbox31;
        public:
            virtual ~MapObject();

            const std::shared_ptr<const ObfMapSectionInfo> section;
            const std::shared_ptr<const ObfMapSectionLevel> level;

            const uint64_t& id;
            const bool& isArea;
            const QVector< PointI >& points31;
            const QList< QVector< PointI > >& innerPolygonsPoints31;
            const QVector< uint32_t >& typesRuleIds;
            const QVector< uint32_t >& extraTypesRuleIds;
            const MapFoundationType& foundation;
            const QMap<uint32_t, QString>& names;
            const AreaI& bbox31;

            int getSimpleLayerValue() const;
            bool isClosedFigure(bool checkInner = false) const;

            bool containsType(const uint32_t typeRuleId, bool checkAdditional = false) const;
            bool containsTypeSlow(const QString& tag, const QString& value, bool checkAdditional = false) const;

            bool intersects(const AreaI& area) const;

            static uint64_t getUniqueId(const std::shared_ptr<const MapObject>& mapObject);
            static uint64_t getUniqueId(const uint64_t id, const std::shared_ptr<const ObfMapSectionInfo>& section);

        friend class OsmAnd::ObfMapSectionReader_P;
        friend class OsmAnd::Rasterizer_P;
        };
    } // namespace Model
} // namespace OsmAnd

#endif // !defined(_OSMAND_CORE_MAP_OBJECT_H_)
