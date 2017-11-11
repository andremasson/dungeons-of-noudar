#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <string>
#include <iostream>
#include <memory>
#include <utility>
#include <map>
#include <functional>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <sstream>
#include <sg14/fixed_point>

using sg14::fixed_point;

#include <EASTL/vector.h>
#include <EASTL/array.h>
#include <NativeBitmap.h>

using eastl::vector;
using eastl::array;
using namespace std::chrono;

#include "IFileLoaderDelegate.h"
#include "NativeBitmap.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "LoadPNG.h"


namespace odb {

    const static bool kShouldDrawOutline = false;

    vector<std::shared_ptr<odb::NativeBitmap>> loadBitmapList(std::string filename, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {
        auto data = fileLoader->loadFileFromPath( filename );
        std::stringstream dataStream;

        dataStream << data;

        std::string buffer;

        vector<std::shared_ptr<odb::NativeBitmap>> toReturn;

        while (dataStream.good()) {
            std::getline(dataStream, buffer);

            toReturn.push_back(loadPNG(buffer, fileLoader));
        }

        return toReturn;
    }


    vector<vector<std::shared_ptr<odb::NativeBitmap>>>
    loadTexturesForLevel(int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {

        std::stringstream roomName("");
        roomName << "tiles";
        roomName << levelNumber;
        roomName << ".lst";
        std::string tilesFilename = roomName.str();
        auto data = fileLoader->loadFileFromPath( tilesFilename );
        std::stringstream dataStream;

        dataStream << data;

        std::string buffer;

        vector<vector<std::shared_ptr<odb::NativeBitmap>>> tilesToLoad;

        while (dataStream.good()) {
            std::getline(dataStream, buffer);
            vector<std::shared_ptr<odb::NativeBitmap>> textures;


            if (buffer.substr(buffer.length() - 4) == ".lst") {
                auto frames = loadBitmapList(buffer, fileLoader );
                for ( const auto frame : frames ) {
                    textures.push_back(frame);
                }
            } else {
                textures.push_back(loadPNG(buffer, fileLoader));
            }

            tilesToLoad.push_back(textures);
        }

        return tilesToLoad;
    }


    void CRenderer::drawMap(Knights::CMap &map, std::shared_ptr<Knights::CActor> current) {
        auto mapCamera = current->getPosition();
//        mCamera = Vec3{ mapCamera.x, 0, mapCamera.y};

        if (!mCached ) {
            mCached = true;
            mNeedsToRedraw = true;
        }
    }

    Knights::CommandType CRenderer::getInput() {
        auto toReturn = mBufferedCommand;
        mBufferedCommand = '.';
        return toReturn;
    }

    Vec2 CRenderer::project(const Vec3&  p ) {
        FixP halfWidth{160};
        FixP halfHeight{100};
        FixP oneOver = divide( halfHeight, p.mZ );

        return {
                halfWidth + multiply(p.mX, oneOver),
                halfHeight - multiply(p.mY, oneOver)
        };
    }

    void CRenderer::projectAllVertices() {
        FixP halfWidth{160};
        FixP halfHeight{100};


        for ( auto& vertex : mVertices ) {

            if (vertex.first.mZ == 0 ) {
                continue;
            }
            FixP oneOver = divide( halfHeight, vertex.first.mZ );

            vertex.second = {
                    halfWidth + multiply(vertex.first.mX, oneOver),
                    halfHeight - multiply(vertex.first.mY, oneOver)
            };
        }
    }

    void CRenderer::drawCubeAt(const Vec3& center, std::shared_ptr<odb::NativeBitmap> texture) {

        if (static_cast<int>(center.mZ) <= 1 ) {
            return;
        }

        FixP one{ 1 };

        mVertices[ 0 ].first = ( center + Vec3{ -one, -one, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one, -one, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one,  one, -one });
        mVertices[ 3 ].first = ( center + Vec3{  one,  one, -one });
        mVertices[ 4 ].first = ( center + Vec3{ -one, -one,  one });
        mVertices[ 5 ].first = ( center + Vec3{  one, -one,  one });
        mVertices[ 6 ].first = ( center + Vec3{ -one,  one,  one });
        mVertices[ 7 ].first = ( center + Vec3{  one,  one,  one });

        projectAllVertices();

        auto ulz0 = mVertices[0].second;
        auto urz0 = mVertices[1].second;
        auto llz0 = mVertices[2].second;
        auto lrz0 = mVertices[3].second;
        auto ulz1 = mVertices[4].second;
        auto urz1 = mVertices[5].second;
        auto llz1 = mVertices[6].second;
        auto lrz1 = mVertices[7].second;

        if (static_cast<int>(center.mX) <= 0 ) {
            drawWall( urz0.mX, urz1.mX,
                      urz0.mY, lrz0.mY,
                      urz1.mY, lrz1.mY,
                      texture);
        }


        if (static_cast<int>(center.mY) >= 0 ) {
            drawFloor(ulz1.mY, urz0.mY,
                      ulz1.mX, urz1.mX,
                      ulz0.mX, urz0.mX,
                      texture);
        }

        if (static_cast<int>(center.mY) <= 0 ) {
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      texture);
        }

        if (static_cast<int>(center.mX) >= 0 ) {
            drawWall(ulz1.mX, ulz0.mX,
                     ulz1.mY, llz1.mY,
                     urz0.mY, lrz0.mY,
                     texture);
        }

        drawWall( ulz0.mX, urz0.mX,
                  ulz0.mY, llz0.mY,
                  urz0.mY, lrz0.mY,
                  texture );



            drawLine(ulz0, ulz1);
            drawLine(llz0, llz1);
            drawLine(urz0, urz1);
            drawLine(lrz0, lrz1);
        }
    }


    /*
     *         /|x1y0
     * x0y0   / |
     *       |  |
     *       |  |
     * x0y1  |  |
     *       \  |
     *        \ |
     *         \| x1y1
     */
    void CRenderer::drawWall( FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1, std::shared_ptr<odb::NativeBitmap> texture ) {

        if ( x0 > x1) {
            //switch x0 with x1
            x0 = x0 + x1;
            x1 = x0 - x1;
            x0 = x0 - x1;

            //switch x0y0 with x1y0
            x0y0 = x0y0 + x1y0;
            x1y0 = x0y0 - x1y0;
            x0y0 = x0y0 - x1y0;

            //switch x0y1 with x1y1
            x0y1 = x0y1 + x1y1;
            x1y1 = x0y1 - x1y1;
            x0y1 = x0y1 - x1y1;
        }

        auto x = static_cast<int16_t >(x0);
        auto limit = static_cast<int16_t >(x1);

        if ( x == limit ) {
            return;
        }

        FixP upperY0 = x0y0;
        FixP lowerY0 = x0y1;
        FixP upperY1 = x1y0;
        FixP lowerY1 = x1y1;

        if ( x0y0 > x0y1 ) {
            upperY0 = x0y1;
            lowerY0 = x0y0;
            upperY1 = x1y1;
            lowerY1 = x1y0;
        };

        FixP upperDy = upperY1 - upperY0;
        FixP lowerDy = lowerY1 - lowerY0;

        FixP y0 = upperY0;
        FixP y1 = lowerY0;

        FixP dX{limit - x};
        FixP upperDyDx = upperDy / dX;
        FixP lowerDyDx = lowerDy / dX;

        uint32_t pixel = 0;

        FixP u{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        //we can use this statically, since the textures are already loaded.
        //we don't need to fetch that data on every run.
        int* data = texture->getPixelData();
        int8_t textureWidth = texture->getWidth();
        FixP textureSize{ textureWidth };

        FixP du = textureSize / (dX);
        auto ix = x;

        for (; ix < limit; ++ix ) {

            FixP dv = textureSize / ( y1 - y0 );
            FixP v{0};
            auto iu = static_cast<uint8_t >(u);

            auto iY0 = static_cast<int16_t >(y0);
            auto iY1 = static_cast<int16_t >(y1);

            for ( auto iy = iY0; iy < iY1; ++iy ) {

                auto iv = static_cast<uint8_t >(v);

                if ( iv != lastV || iu != lastU ) {
                    pixel = data[ (iv * textureWidth ) + iu];
                }

                lastU = iu;
                lastV = iv;

                if ( pixel & 0xFF000000 ) {
                    putRaw( ix, iy, pixel);
                }

                v += dv;
            }

            y0 += upperDyDx;
            y1 += lowerDyDx;
            u += du;
        }

    }


    /*
     *     x0y0 ____________ x1y0
     *         /            \
     *        /             \
     *  x0y1 /______________\ x1y1
     */
    void CRenderer::drawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1, std::shared_ptr<odb::NativeBitmap> texture ) {

        //if we have a trapezoid in which the base is smaller
        if ( y0 > y1) {
            //switch y0 with y1
            y0 = y0 + y1;
            y1 = y0 - y1;
            y0 = y0 - y1;

            //switch x0y0 with x0y1
            x0y0 = x0y0 + x0y1;
            x0y1 = x0y0 - x0y1;
            x0y0 = x0y0 - x0y1;

            //switch x1y0 with x1y1
            x1y0 = x1y0 + x1y1;
            x1y1 = x1y0 - x1y1;
            x1y0 = x1y0 - x1y1;
        }

        auto y = static_cast<int16_t >(y0);
        auto limit = static_cast<int16_t >(y1);

        if ( y == limit ) {
            return;
        }

        FixP upperX0 = x0y0;
        FixP upperX1 = x1y0;
        FixP lowerX0 = x0y1;
        FixP lowerX1 = x1y1;

        //what if the trapezoid is flipped horizontally?
        if ( x0y0 > x1y0 ) {
            upperX0 = x1y0;
            upperX1 = x0y0;
            lowerX0 = x1y1;
            lowerX1 = x0y1;
        };

        FixP leftDX = lowerX0 - upperX0;
        FixP rightDX = lowerX1 - upperX1;
        FixP dY = y1 - y0;
        FixP leftDxDy = leftDX / dY;
        FixP rightDxDy = rightDX / dY;
        FixP x0 = upperX0;
        FixP x1 = upperX1;

        uint32_t pixel = 0 ;

        FixP v{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        auto iy = static_cast<int16_t >(y);

        int* data = texture->getPixelData();
        int8_t textureWidth = texture->getWidth();
        FixP textureSize{ textureWidth };

        FixP dv = textureSize / (dY);

        for (; iy < limit; ++iy ) {

            auto iX0 = static_cast<int16_t >(x0);
            auto iX1 = static_cast<int16_t >(x1);

            FixP du = textureSize / ( x1 - x0 );
            FixP u{0};
            auto iv = static_cast<uint8_t >(v);

            for ( auto ix = iX0; ix < iX1; ++ix ) {

                auto iu = static_cast<uint8_t >(u);

                //only fetch the next texel if we really changed the u, v coordinates
                //(otherwise, would fetch the same thing anyway)
                if ( iv != lastV || iu != lastU ) {
                    pixel = data[ (iv * textureWidth ) + iu];
                }

                lastU = iu;
                lastV = iv;

                if ( pixel & 0xFF000000 ) {
                    putRaw( ix, iy, pixel);
                }

                u += du;
            }

            x0 += leftDxDy;
            x1 += rightDxDy;

            v += dv;
        }
    }

    void CRenderer::drawLine(const Vec2& p0, const Vec2& p1) {
        drawLine(static_cast<int16_t >(p0.mX),
                 static_cast<int16_t >(p0.mY),
                 static_cast<int16_t >(p1.mX),
                 static_cast<int16_t >(p1.mY)
        );
    }

    void CRenderer::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {

        if ( x0 == x1 ) {

            int16_t _y0 = y0;
            int16_t _y1 = y1;

            if ( y0 > y1 ) {
                _y0 = y1;
                _y1 = y0;
            }

            for ( int y = _y0; y <= _y1; ++y ) {
                putRaw( x0, y, 0xFFFFFFFF);
            }
            return;
        }

        if ( y0 == y1 ) {
            int16_t _x0 = x0;
            int16_t _x1 = x1;

            if ( x0 > x1 ) {
                _x0 = x1;
                _x1 = x0;
            }

            for ( int x = _x0; x <= _x1; ++x ) {
                putRaw( x, y0, 0xFFFFFFFF);
            }
            return;
        }

        //switching x0 with x1
        if( x0 > x1 ) {
            x0 = x0 + x1;
            x1 = x0 - x1;
            x0 = x0 - x1;

            y0 = y0 + y1;
            y1 = y0 - y1;
            y0 = y0 - y1;
        }

        FixP fy = FixP{ y0 };
        FixP fLimitY = FixP{ y1 };
        FixP fDeltatY = FixP{ y1 - y0 } / FixP{ x1 - x0 };

        for ( int x = x0; x <= x1; ++x ) {
            putRaw( x, static_cast<int16_t >(fy), 0xFFFFFFFF);
            fy += fDeltatY;
        }
    }

    void CRenderer::render(long ms) {

        const static FixP two{2};

        if ( mSpeed.mX ) {
            mSpeed.mX /= two;
            mNeedsToRedraw = true;
        }

        if  ( mSpeed.mY ) {
            mSpeed.mY /= two;
            mNeedsToRedraw = true;
        }

        if  ( mSpeed.mZ ) {
            mSpeed.mZ /= two;
            mNeedsToRedraw = true;
        }

        mCamera += mSpeed;

        if ( mNeedsToRedraw ) {
            mNeedsToRedraw = false;

            if ( clearScr ) {
                clear();
            }

            //milliseconds ms0 = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

            drawCubeAt( mCamera + Vec3{ FixP{-4}, FixP{ 1}, FixP{16}}, mTextures[ 0 ][ 0 ] );
            drawCubeAt( mCamera + Vec3{ FixP{ 2}, FixP{ 1}, FixP{ 8}}, mTextures[ 1 ][ 0 ] );
            drawCubeAt( mCamera + Vec3{ FixP{-2}, FixP{-3}, FixP{ 1}}, mTextures[ 2 ][ 0 ] );

            //milliseconds ms1 = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

            //std::cout << "took " << ( ms1 - ms0 ).count() << std::endl;
        }

        flip();
    }

    void CRenderer::loadTextures(vector<vector<std::shared_ptr<odb::NativeBitmap>>> textureList, CTilePropertyMap &tile3DProperties) {
        mTextures = textureList;
        mTileProperties = tile3DProperties;
    }
}
