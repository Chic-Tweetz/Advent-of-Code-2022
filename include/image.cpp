// From https://www.youtube.com/watch?v=vqT5j38bWGg

#include "image.h"

#include <fstream>
#include <iostream>

Color::Color() :
	r{ 0 }, g{ 0 }, b{ 0 }
{
}

Color::Color(float r, float g, float b) :
	r{ r }, g{ g }, b{ b }
{
}

Color::~Color()
{
}

Image::Image(int width, int height) :
	m_width{ width }, m_height{ height }, m_colors{ std::vector<Color>(static_cast<size_t>(width) * static_cast<size_t>(height)) }
{

}

Image::Image(int width, int height, const Color& bg) :
	m_width{ width }, m_height{ height }, m_colors{ std::vector<Color>(static_cast<size_t>(width) * static_cast<size_t>(height), bg)}
{

}


Image::~Image()
{

}

Color Image::getColor(int x, int y)
{
	return m_colors[static_cast<size_t>(y) * static_cast<size_t>(m_width) + static_cast<size_t>(x)];
}

void Image::setColor(const Color& color, int x, int y)
{
	m_colors[static_cast<size_t>(static_cast<size_t>(y) * static_cast<size_t>(m_width) + static_cast<size_t>(x))] = color;
}

void Image::save(const char* path)
{
	std::ofstream f;
	f.open(path, std::ios::out | std::ios::binary);

	if (!f.is_open())
	{
		std::cerr << "File " << path << " could not be opened for writing\n";
		return;
	}

	unsigned char bmpPad[3] = { 0, 0, 0 };
	const int paddingAmount = ((4 - (m_width * 3) % 4) % 4);

	const int fileHeaderSize = 14;
	const int informationHeaderSize = 40;
	const int fileSize = fileHeaderSize + informationHeaderSize + m_width * m_height * 3 + paddingAmount * m_height;

	unsigned char fileHeader[fileHeaderSize]{};

	// File type
	fileHeader[0] = 'B';
	fileHeader[1] = 'M';
	// File size
	fileHeader[2] = static_cast<unsigned char>( fileSize );
	fileHeader[3] = static_cast<unsigned char>( fileSize >> 8 );
	fileHeader[4] = static_cast<unsigned char>( fileSize >> 16 );
	fileHeader[5] = static_cast<unsigned char>( fileSize >> 24 );
	// Reserved 1 (not used)
	fileHeader[6] = 0;
	fileHeader[7] = 0;
	// Reserverd 2 (not used)
	fileHeader[8] = 0;
	fileHeader[9] = 0;
	// Pixel data offset
	fileHeader[10] = fileHeaderSize + informationHeaderSize;
	fileHeader[11] = 0;
	fileHeader[12] = 0;
	fileHeader[13] = 0;

	unsigned char informationHeader[informationHeaderSize]{};

	// Header size
	informationHeader[0] = informationHeaderSize;
	informationHeader[1] = 0;
	informationHeader[2] = 0;
	informationHeader[3] = 0;
	// Image width
	informationHeader[4] = static_cast<unsigned char>(m_width);
	informationHeader[5] = static_cast<unsigned char>(m_width >> 8);
	informationHeader[6] = static_cast<unsigned char>(m_width >> 16);
	informationHeader[7] = static_cast<unsigned char>(m_width >> 24);
	// Image height
	informationHeader[8] = static_cast<unsigned char>(m_height);
	informationHeader[9] = static_cast<unsigned char>(m_height >> 8);
	informationHeader[10] = static_cast<unsigned char>(m_height >> 16);
	informationHeader[11] = static_cast<unsigned char>(m_height >> 24);
	// Planes
	informationHeader[12] = 1;
	informationHeader[13] = 0;
	// Bits per pixel (RGB)
	informationHeader[14] = 24;
	informationHeader[15] = 0;
	// For loop zeroes the following:
	// Compression (no compression)
	// Image size (no compression)
	// X pixels per meter (unspecified)
	// Y pixels per meter (unspecified)
	// Total colors (unspecified)
	// Important colours (generally ignored)
	for (size_t i{ 16 }; i < 40; ++i)
	{
		informationHeader[i] = 0;
	}

	f.write(reinterpret_cast<char*>(fileHeader), fileHeaderSize);
	f.write(reinterpret_cast<char*>(informationHeader), informationHeaderSize);
	
	for (int y{ 0 }; y < m_height; ++y)
	{
		for (int x{ 0 }; x < m_width; ++x)
		{
			unsigned char r = static_cast<unsigned char>(getColor(x, y).r * 255.0f);
			unsigned char g = static_cast<unsigned char>(getColor(x, y).g * 255.0f);
			unsigned char b = static_cast<unsigned char>(getColor(x, y).b * 255.0f);

			unsigned char color[] = { b, g, r };

			f.write(reinterpret_cast<char*>(color), 3);
		}

		f.write(reinterpret_cast<char*>(bmpPad), paddingAmount);
	}

	f.close();

	std::cout << "File: " << path << " created\n";
}

