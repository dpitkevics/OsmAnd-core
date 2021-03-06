#include "ObfsCollection_P.h"
#include "ObfsCollection.h"

#include <cassert>
#if OSMAND_DEBUG
#   include <chrono>
#endif

#include "ObfReader.h"
#include "ObfDataInterface.h"
#include "ObfFile.h"
#include "QKeyValueIterator.h"
#include "Utilities.h"
#include "Logging.h"

OsmAnd::ObfsCollection_P::ObfsCollection_P(ObfsCollection* owner_)
    : owner(owner_)
    , _lastUnusedEntryId(0)
    , _collectedSourcesInvalidated(1)
    , _sourcesSetModifier(nullptr)
{
}

OsmAnd::ObfsCollection_P::~ObfsCollection_P()
{
}

void OsmAnd::ObfsCollection_P::invalidateCollectedSources()
{
    if(!_collectedSourcesInvalidated.testAndSetOrdered(-1, 1))
        _collectedSourcesInvalidated.fetchAndAddOrdered(1);
}

void OsmAnd::ObfsCollection_P::collectSources()
{
    QWriteLocker scopedLocker1(&_collectedSourcesLock);
    QReadLocker scopedLocker2(&_entriesLock);

#if OSMAND_DEBUG
    const auto collectSources_Begin = std::chrono::high_resolution_clock::now();
#endif

    // For each collection by EntryId, ...
    QMutableHashIterator< ObfsCollection::EntryId, QHash<QString, std::shared_ptr<ObfFile> > > itCollectedSourcesEntry(_collectedSources);
    while(itCollectedSourcesEntry.hasNext())
    {
        const auto& originId = itCollectedSourcesEntry.key();
        auto& collectedSources = itCollectedSourcesEntry.value();

        // If this entry no longer exists, remove it entirely
        if(!_entries.contains(originId))
        {
            itCollectedSourcesEntry.remove();
            _collectedSourcesOriginRefCounters.remove(originId);

            continue;
        }

        // For each file in registry, ...
        QMutableHashIterator< QString, std::shared_ptr<ObfFile> > itObfFileEntry(collectedSources);
        while(itObfFileEntry.hasNext())
        {
            const auto& sourceFilename = itObfFileEntry.next().key();

            // ... which does not exist, ...
            if(QFile::exists(sourceFilename))
                continue;

            // ... remove entry
            itObfFileEntry.remove();
        }

        // If all files are gone 
        if(collectedSources.isEmpty())
        {
            itCollectedSourcesEntry.remove();
            _collectedSourcesOriginRefCounters.remove(originId);

            continue;
        }
    }

    // Find all files that are present in registered entries and add them (if they are missing)
    for(const auto& itEntry : rangeOf(constOf(_entries)))
    {
        const auto& originId = itEntry.key();
        const auto& entry = itEntry.value();
        auto itCollectedSources = _collectedSources.find(originId);
        if(itCollectedSources == _collectedSources.end())
        {
            itCollectedSources = _collectedSources.insert(originId, QHash<QString, std::shared_ptr<ObfFile> >());
            _collectedSourcesOriginRefCounters.insert(originId, QAtomicInt(0));
        }
        auto& collectedSources = *itCollectedSources;

        if(entry->type == EntryType::DirectoryEntry)
        {
            const auto& directoryEntry = std::static_pointer_cast<DirectoryEntry>(entry);

            QFileInfoList obfFilesInfo;
            Utilities::findFiles(directoryEntry->dir, QStringList() << QLatin1String("*.obf"), obfFilesInfo, directoryEntry->recursive);
            for(const auto& obfFileInfo : constOf(obfFilesInfo))
            {
                const auto& obfFilePath = obfFileInfo.canonicalFilePath();
                auto itObfFileEntry = collectedSources.constFind(obfFilePath);

                // ... which is not yet present in registry, ...
                if(itObfFileEntry == collectedSources.cend())
                {
                    // ... create ObfFile
                    auto obfFile = new ObfFile(obfFilePath);
                    itObfFileEntry = collectedSources.insert(obfFilePath, std::shared_ptr<ObfFile>(obfFile));
                }
            }
        }
        else if(entry->type == EntryType::ExplicitFileEntry)
        {
            const auto& explicitFileEntry = std::static_pointer_cast<ExplicitFileEntry>(entry);

            if(explicitFileEntry->fileInfo.exists())
            {
                const auto& obfFilePath = explicitFileEntry->fileInfo.canonicalFilePath();
                auto itObfFileEntry = collectedSources.constFind(obfFilePath);

                // ... which is not yet present in registry, ...
                if(itObfFileEntry == collectedSources.cend())
                {
                    // ... create ObfFile
                    auto obfFile = new ObfFile(obfFilePath);
                    itObfFileEntry = collectedSources.insert(obfFilePath, std::shared_ptr<ObfFile>(obfFile));
                }
            }
        }
    }

#if OSMAND_DEBUG
    const auto collectSources_End = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float> collectSources_Elapsed = collectSources_End - collectSources_Begin;
    LogPrintf(LogSeverityLevel::Info, "Collected OBF sources in %fs", collectSources_Elapsed.count());
#endif
}

OsmAnd::ObfsCollection::EntryId OsmAnd::ObfsCollection_P::registerDirectory(const QDir& dir, bool recursive /*= true*/)
{
    QWriteLocker scopedLocker(&_entriesLock);

    const auto allocatedId = _lastUnusedEntryId++;
    auto entry = new DirectoryEntry();
    entry->dir = dir;
    entry->recursive = recursive;
    _entries.insert(allocatedId, qMove(std::shared_ptr<Entry>(entry)));

    invalidateCollectedSources();

    return allocatedId;
}

OsmAnd::ObfsCollection::EntryId OsmAnd::ObfsCollection_P::registerExplicitFile(const QFileInfo& fileInfo)
{
    QWriteLocker scopedLocker(&_entriesLock);

    const auto allocatedId = _lastUnusedEntryId++;
    auto entry = new ObfsCollection_P::ExplicitFileEntry();
    entry->fileInfo = fileInfo;
    _entries.insert(allocatedId, qMove(std::shared_ptr<Entry>(entry)));

    invalidateCollectedSources();

    return allocatedId;
}

bool OsmAnd::ObfsCollection_P::unregister(const ObfsCollection::EntryId entryId)
{
    //TODO: can not unregister if entry is used in any active data interface
    QWriteLocker scopedLocker(&_entriesLock);

    const auto wasRemoved = (_entries.remove(entryId) > 0);
    if(wasRemoved)
        invalidateCollectedSources();
    return wasRemoved;
}

void OsmAnd::ObfsCollection_P::notifyFileSystemChange()
{
    invalidateCollectedSources();
}

QStringList OsmAnd::ObfsCollection_P::getCollectedSources() const
{
    QReadLocker scopedLocker(&_collectedSourcesUpdateObserversLock);

    QStringList result;
    for(const auto& collectedSources : constOf(_collectedSources))
        result << collectedSources.keys();
    return result;
}

void OsmAnd::ObfsCollection_P::notifyCollectedSourcesUpdate() const
{
    QHash<void*, ObfsCollection::CollectedSourcesUpdateObserverSignature> collectedSourcesUpdateObservers;
    {
        QReadLocker scopedLocker(&_collectedSourcesUpdateObserversLock);
        collectedSourcesUpdateObservers = _collectedSourcesUpdateObservers;
        collectedSourcesUpdateObservers.detach();
    }

    for(const auto& observer : constOf(collectedSourcesUpdateObservers))
    {
        if(observer)
            observer(*owner);
    }
}

void OsmAnd::ObfsCollection_P::registerCollectedSourcesUpdateObserver(void* tag, const ObfsCollection::CollectedSourcesUpdateObserverSignature observer)
{
    QWriteLocker scopedLocker(&_collectedSourcesUpdateObserversLock);

    _collectedSourcesUpdateObservers.insert(tag, observer);
}

void OsmAnd::ObfsCollection_P::unregisterCollectedSourcesUpdateObserver(void* tag)
{
    QWriteLocker scopedLocker(&_collectedSourcesUpdateObserversLock);

    _collectedSourcesUpdateObservers.remove(tag);
}

void OsmAnd::ObfsCollection_P::setSourcesSetModifier(const ObfsCollection::SourcesSetModifierSignature modifier)
{
    QWriteLocker scopedLocker(&_sourcesSetModifierLock);

    _sourcesSetModifier = modifier;
}

std::shared_ptr<OsmAnd::ObfDataInterface> OsmAnd::ObfsCollection_P::obtainDataInterface()
{
    // Check if sources were invalidated
    if(_collectedSourcesInvalidated.fetchAndStoreOrdered(0) != 0)
    {
        collectSources();
        notifyCollectedSourcesUpdate();
    }

    // Create ObfReader's
    QList< std::shared_ptr<ObfReader> > obfReaders;
    QSet< ObfsCollection::EntryId > referencedOrigins;
    {
        QReadLocker scopedLocker(&_collectedSourcesLock);

        for(const auto& itCollectedSources : rangeOf(constOf(_collectedSources)))
        {
            const auto& originId = itCollectedSources.key();
            const auto& collectedSources = itCollectedSources.value();

            // Increment reference counter of origin
            referencedOrigins.insert(originId);
            _collectedSourcesOriginRefCounters[originId].fetchAndAddOrdered(1);

            obfReaders.reserve(obfReaders.size() + collectedSources.size());
            for(const auto& obfFile : constOf(collectedSources))
            {
                auto obfReader = new ObfReader(obfFile);
                obfReaders.push_back(qMove(std::shared_ptr<ObfReader>(obfReader)));
            }
        }
    }

    // Allow modifier to modify set of sources prior to final use
    ObfsCollection::SourcesSetModifierSignature sourcesSetModifier;
    {
        QReadLocker scopedLocker(&_sourcesSetModifierLock);

        sourcesSetModifier = _sourcesSetModifier;
    }
    if(sourcesSetModifier != nullptr)
        sourcesSetModifier(*owner, obfReaders);

    return std::shared_ptr<ObfDataInterface>(new ObfDataInterface(obfReaders), [this, referencedOrigins](ObfDataInterface* object)
    {
        {
            QReadLocker scopedLocker(&_collectedSourcesLock);

            // Dereference all referenced origins
            for(const auto& id : constOf(referencedOrigins))
            {
                _collectedSourcesOriginRefCounters[id].fetchAndAddOrdered(-1);
                assert(_collectedSourcesOriginRefCounters[id].load() >= 0);
            }
        }

        delete object;
    });
}
