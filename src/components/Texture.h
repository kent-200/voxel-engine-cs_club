#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb_image.h"


struct Texture {
    unsigned int id;    // gets assigned when loaded
    std::string path;

    // for texture atlas, number of rows and columns
    // assume all assets are same size
    unsigned int atlasRows;
    unsigned int atlasCols;


    Texture(std::string path, unsigned int atlasRows, unsigned int atlasCols);

    void loadTexture();
    void bindTexture(unsigned int unit);

    ~Texture();
};


Texture::Texture(std::string path, unsigned int atlasRows = 1,
                  unsigned int atlasCols = 1) {
    this->path = path;
    this->atlasRows = atlasRows;
    this->atlasCols = atlasCols;

    loadTexture();
}


// load texture from file
void Texture::loadTexture() {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    
    // Set the texture wrapping parameters, chose pixelated look
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // repeated setting - want to stretch on both x any y axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height,
                                    &nrChannels, 0); // stbi_image_free(data);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
}


// if using multiple textures, each needs its own texture unit
void Texture::bindTexture(unsigned int unit = 0) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}

Texture::~Texture() {
    glDeleteTextures(1, &id);
}


#endif