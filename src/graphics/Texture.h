#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

class Texture {
public:
    unsigned int ID;
    int width, height, nrChannels;

    Texture(const std::string& path);
    ~Texture();

    void Bind(unsigned int unit = 0) const;
    void Unbind() const;
};

#endif