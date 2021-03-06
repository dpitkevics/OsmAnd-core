#include "HeightmapTileProvider.h"
#include "HeightmapTileProvider_P.h"

const QString OsmAnd::HeightmapTileProvider::defaultIndexFilename("heightmap.index");

OsmAnd::HeightmapTileProvider::HeightmapTileProvider( const QDir& dataPath, const QString& indexFilepath/* = QString()*/ )
    : _d(new HeightmapTileProvider_P(this, dataPath, indexFilepath))
{
}

OsmAnd::HeightmapTileProvider::~HeightmapTileProvider()
{
}

void OsmAnd::HeightmapTileProvider::rebuildTileDbIndex()
{
    _d->_tileDb.rebuildIndex();
}

uint32_t OsmAnd::HeightmapTileProvider::getTileSize() const
{
    return 32;
}

bool OsmAnd::HeightmapTileProvider::obtainTile(const TileId tileId, const ZoomLevel zoom, std::shared_ptr<const MapTile>& outTile, const IQueryController* const queryController)
{
    return _d->obtainTile(tileId, zoom, outTile, queryController);
}

OsmAnd::ZoomLevel OsmAnd::HeightmapTileProvider::getMinZoom() const
{
    return _d->getMinZoom();
}

OsmAnd::ZoomLevel OsmAnd::HeightmapTileProvider::getMaxZoom() const
{
    return _d->getMaxZoom();
}
