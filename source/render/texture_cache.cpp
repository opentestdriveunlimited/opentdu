#include "shared.h"
#include "texture_cache.h"

#include "2db.h"

TextureCache gTextureCache = {};

TextureCache::TextureCache()
{

}

TextureCache::~TextureCache()
{
    
}

Texture* TextureCache::getTexture(const char* pName)
{
    uint64_t hashcode = GetIdentifier64bit(pName);
    return getTexture(hashcode);
}

Texture* TextureCache::getTexture(const uint64_t hashcode)
{
    auto it = cache.find(hashcode);
    return it != cache.end() ? it->second : nullptr;
}

bool TextureCache::insertIntoCache(Texture* pTexture)
{
    OTDU_ASSERT( pTexture->Hashcode != 0ull );

    if (getTexture(pTexture->Hashcode) != nullptr) {
        OTDU_LOG_ERROR("Failed to register %ull (already in cache!)\n", pTexture->Hashcode);
        return false;
    }

    cache.insert(std::make_pair(pTexture->Hashcode, pTexture));
    return true;
}

void TextureCache::insertAndUpload(Texture* pTexture, const bool bForce)
{
    if (bForce || !pTexture->bUploaded) {
        Render2DB::PrepareTexture(pTexture);
        insertIntoCache(pTexture);
    }
}
