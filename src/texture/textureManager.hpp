#include <unordered_map>
#include <string>

namespace rtk {
    class TextureManager {
    private:
        std::unordered_map<std::string, uint32_t> _textureCache;
        
    public:
        uint32_t loadTexture(const std::string& filepath) {
            auto it = _textureCache.find(filepath);

            if (it != _textureCache.end())
                return it->second;

            uint32_t newTextureId = createtexture();
            
            _textureCache[filepath] = newTextureId;
            
            return newTextureId;
        }
    };
}