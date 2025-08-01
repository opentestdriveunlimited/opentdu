#pragma once

struct Texture;

class TextureCache {
public:
    TextureCache();
    ~TextureCache();

    Texture* getTexture(const char* pName);
    Texture* getTexture(const uint64_t hashcode);

    bool insertIntoCache(Texture* pTexture);
    void insertAndUpload(Texture* pTexture, const bool bForce = false);

private:
    std::unordered_map<uint64_t, Texture*> cache;
};

extern TextureCache gTextureCache;
