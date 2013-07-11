#include "HeightmapTileProvider.h"

#include <assert.h>

#include "Concurrent.h"

const QString OsmAnd::HeightmapTileProvider::defaultIndexFilename("heightmap.index");

OsmAnd::HeightmapTileProvider::HeightmapTileProvider( const QDir& dataPath, const QString& indexFilepath/* = QString()*/ )
    : _tileDb(dataPath, indexFilepath)
    , tileDb(_tileDb)
{
}

OsmAnd::HeightmapTileProvider::~HeightmapTileProvider()
{
    //TODO: on destruction, cancel all tasks
}

void OsmAnd::HeightmapTileProvider::rebuildTileDbIndex()
{
    _tileDb.rebuildIndex();
}

uint32_t OsmAnd::HeightmapTileProvider::getTileSize() const
{
    return 258;
}

bool OsmAnd::HeightmapTileProvider::obtainTileImmediate( const TileId& tileId, uint32_t zoom, std::shared_ptr<IMapTileProvider::Tile>& tile )
{
    // Heightmap tiles are not available immediately, since none of them are stored in memory unless they are just
    // downloaded. In that case, a callback will be called
    return false;
}

void OsmAnd::HeightmapTileProvider::obtainTileDeffered( const TileId& tileId, uint32_t zoom, TileReadyCallback readyCallback )
{
    assert(readyCallback != nullptr);

    {
        QMutexLocker scopeLock(&_requestsMutex);
        if(_requestedTileIds[zoom].contains(tileId))
            return;

        _requestedTileIds[zoom].insert(tileId);
    }

    Concurrent::instance()->localStoragePool->start(new Concurrent::Task(
        [this, tileId, zoom, readyCallback](const Concurrent::Task* task, QEventLoop& eventLoop)
        {
            _processingMutex.lock();

            // Obtain raw data from DB
            QByteArray data;
            bool ok = _tileDb.obtainTileData(tileId, zoom, data);
            if(!ok || data.length() == 0)
            {
                {
                    QMutexLocker scopeLock(&_requestsMutex);
                    _requestedTileIds[zoom].remove(tileId);
                }
                _processingMutex.unlock();

                // There was no data at all, to avoid further requests, mark this tile as empty
                std::shared_ptr<IMapTileProvider::Tile> emptyTile;
                readyCallback(tileId, zoom, emptyTile, true);

                return;
            }

            // We have the data, use GDAL to decode this GeoTIFF
            assert(false);

            // Construct tile response
            {
                QMutexLocker scopeLock(&_requestsMutex);
                _requestedTileIds[zoom].remove(tileId);
            }
            _processingMutex.unlock();

            std::shared_ptr<IMapTileProvider::Tile> tile;//(new Tile());
            readyCallback(tileId, zoom, tile, true);
        }));
}

OsmAnd::HeightmapTileProvider::Tile::Tile( const float* data, size_t rowLength, uint32_t width, uint32_t height )
    : IMapElevationDataProvider::Tile(data, rowLength, width, height)
{
}

OsmAnd::HeightmapTileProvider::Tile::~Tile()
{
}
