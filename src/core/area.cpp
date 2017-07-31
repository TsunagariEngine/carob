/***************************************
** Tsunagari Tile Engine              **
** area.cpp                           **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2016 Paul Merrill   **
***************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#include "core/area.h"

#include <math.h>

#include <algorithm>

#include "core/algorithm.h"
#include "core/client-conf.h"
#include "core/entity.h"
#include "core/formatter.h"
#include "core/log.h"
#include "core/images.h"
#include "core/music.h"
#include "core/npc.h"
#include "core/overlay.h"
#include "core/player.h"
#include "core/tile.h"
#include "core/viewport.h"
#include "core/window.h"
#include "core/world.h"
#include "util/assert.h"
#include "util/math2.h"

#include "data/data-world.h"

#define CHECK(x)  if (!(x)) { return false; }

/* NOTE: In the TMX map format used by Tiled, tileset tiles start counting
         their Y-positions from 0, while layer tiles start counting from 1. I
         can't imagine why the author did this, but we have to take it into
         account.
*/

Area::Area(Player* player,
           const std::string& descriptor)
    : dataArea(DataWorld::instance().area(descriptor)),
      player(player),
      colorOverlayARGB(0),
      beenFocused(false),
      redraw(true),
      descriptor(descriptor)
{
    grid.dim = ivec3(0, 0, 0);
    grid.tileDim = ivec2(0, 0);
    grid.loopX = false;
    grid.loopY = false;
}

Area::~Area()
{
}

bool Area::init()
{
    // Abstract method.
    return false;
}

void Area::focus()
{
    if (!beenFocused) {
        beenFocused = true;
        if (dataArea) {
            dataArea->onLoad();
        }
    }

    if (musicPath) {
        Music::instance().play(*musicPath);
    }

    if (dataArea) {
        dataArea->onFocus();
    }
}

void Area::buttonDown(KeyboardKey key)
{
    switch (key) {
    case KBLeftArrow:
        player->startMovement(ivec2(-1, 0));
        break;
    case KBRightArrow:
        player->startMovement(ivec2(1, 0));
        break;
    case KBUpArrow:
        player->startMovement(ivec2(0, -1));
        break;
    case KBDownArrow:
        player->startMovement(ivec2(0, 1));
        break;
    case KBSpace:
        player->useTile();
        break;
    default:
        break;
    }
}

void Area::buttonUp(KeyboardKey key)
{
    switch (key) {
    case KBLeftArrow:
        player->stopMovement(ivec2(-1, 0));
        break;
    case KBRightArrow:
        player->stopMovement(ivec2(1, 0));
        break;
    case KBUpArrow:
        player->stopMovement(ivec2(0, -1));
        break;
    case KBDownArrow:
        player->stopMovement(ivec2(0, 1));
        break;
    default:
        break;
    }
}

void Area::draw()
{
    drawTiles();
    drawEntities();
    redraw = false;
}

bool Area::needsRedraw() const
{
    if (redraw) {
        return true;
    }

    const icube tiles = visibleTiles();
    const icube pixels = {
        tiles.x1 * grid.tileDim.x,
        tiles.y1 * grid.tileDim.y,
        tiles.z1,
        tiles.x2 * grid.tileDim.x,
        tiles.y2 * grid.tileDim.y,
        tiles.z2,
    };

    if (player->needsRedraw(pixels)) {
        return true;
    }
    for (const auto& character : characters) {
        if (character->needsRedraw(pixels)) {
            return true;
        }
    }
    for (const auto& overlay : overlays) {
        if (overlay->needsRedraw(pixels)) {
            return true;
        }
    }

    // Do any on-screen tile types need to update their animations?
    for (int z = tiles.z1; z < tiles.z2; z++) {
        for (int y = tiles.y1; y < tiles.y2; y++) {
            for (int x = tiles.x1; x < tiles.x2; x++) {
                const Tile* tile = getTile(x, y, z);
                const TileType* type = tile->getType();
                if (type && type->needsRedraw()) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Area::requestRedraw()
{
    redraw = true;
}

void Area::tick(time_t dt)
{
    if (dataArea) {
        dataArea->tick(dt);
    }

    for (auto& overlay : overlays) {
        overlay->tick(dt);
    }
    erase_if(overlays, [] (const Rc<Overlay>& o) {
        bool dead = o->isDead();
        if (dead) {
            o->setArea(nullptr);
        }
        return dead;
    });

    if (conf.moveMode != TURN) {
        player->tick(dt);

        for (auto& character : characters) {
            character->tick(dt);
        }
        erase_if(characters, [] (const Rc<Character>& c) {
            bool dead = c->isDead();
            if (dead) {
                c->setArea(nullptr);
            }
            return dead;
        });
    }

    Viewport::instance().tick(dt);
}

void Area::turn()
{
    if (dataArea) {
        dataArea->turn();
    }

    player->turn();

    for (auto& character : characters) {
        character->turn();
    }
    erase_if(characters, [] (const Rc<Character>& c) {
        bool dead = c->isDead();
        if (dead) {
            c->setArea(nullptr);
        }
        return dead;
    });

    Viewport::instance().turn();
}


uint32_t Area::getColorOverlay()
{
    return colorOverlayARGB;
}

void Area::setColorOverlay(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    colorOverlayARGB = (uint32_t)(a << 24) + (uint32_t)(r << 16) +
        (uint32_t)(g << 8) + (uint32_t)b;
    redraw = true;
}


const Tile* Area::getTile(int x, int y, int z) const
{
    return grid.getTile(icoord(x, y, z));
}

const Tile* Area::getTile(int x, int y, double z) const
{
    return grid.getTile(vicoord(x, y, z));
}

const Tile* Area::getTile(icoord phys) const
{
    return grid.getTile(phys);
}

const Tile* Area::getTile(vicoord virt) const
{
    return grid.getTile(virt);
}

const Tile* Area::getTile(rcoord virt) const
{
    return grid.getTile(virt);
}

Tile* Area::getTile(int x, int y, int z)
{
    return grid.getTile(icoord(x, y, z));
}

Tile* Area::getTile(int x, int y, double z)
{
    return grid.getTile(vicoord(x, y, z));
}

Tile* Area::getTile(icoord phys)
{
    return grid.getTile(phys);
}

Tile* Area::getTile(vicoord virt)
{
    return grid.getTile(virt);
}

Tile* Area::getTile(rcoord virt)
{
    return grid.getTile(virt);
}

TileSet* Area::getTileSet(const std::string& imagePath)
{
    std::map<std::string, TileSet>::iterator it;
    it = tileSets.find(imagePath);
    if (it == tileSets.end()) {
        Log::err("Area", "tileset " + imagePath + " not found");
        return nullptr;
    }
    return &tileSets[imagePath];
}


ivec3 Area::getDimensions() const
{
    return grid.dim;
}

ivec2 Area::getTileDimensions() const
{
    return grid.tileDim;
}

icube Area::visibleTiles() const
{
    Viewport& viewport = Viewport::instance();

    rvec2 screen = viewport.getVirtRes();
    rvec2 off = viewport.getMapOffset();

    int x1 = static_cast<int>(floor(off.x / grid.tileDim.x));
    int y1 = static_cast<int>(floor(off.y / grid.tileDim.y));
    int x2 = static_cast<int>(ceil((screen.x + off.x) / grid.tileDim.x));
    int y2 = static_cast<int>(ceil((screen.y + off.y) / grid.tileDim.y));

    if (!grid.loopX) {
        x1 = bound(x1, 0, grid.dim.x);
        x2 = bound(x2, 0, grid.dim.x);
    }
    if (!grid.loopY) {
        y1 = bound(y1, 0, grid.dim.y);
        y2 = bound(y2, 0, grid.dim.y);
    }

    return icube{x1, y1, 0, x2, y2, grid.dim.z};
}

bool Area::inBounds(int x, int y, int z) const
{
    return grid.inBounds(icoord(x, y, z));
}

bool Area::inBounds(int x, int y, double z) const
{
    return grid.inBounds(vicoord(x, y, z));
}

bool Area::inBounds(icoord phys) const
{
    return grid.inBounds(phys);
}

bool Area::inBounds(vicoord virt) const
{
    return grid.inBounds(virt);
}

bool Area::inBounds(rcoord virt) const
{
    return grid.inBounds(virt);
}

bool Area::inBounds(Entity* ent) const
{
    return inBounds(ent->getPixelCoord());
}



bool Area::loopsInX() const
{
    return grid.loopX;
}

bool Area::loopsInY() const
{
    return grid.loopY;
}

const std::string Area::getDescriptor() const
{
    return descriptor;
}

Rc<NPC> Area::spawnNPC(const std::string& descriptor,
    vicoord coord, const std::string& phase)
{
    auto c = Rc<NPC>(new NPC);
    if (!c->init(descriptor, phase)) {
        // Error logged.
        return Rc<NPC>();
    }
    c->setArea(this);
    c->setTileCoords(coord);
    insert(c);
    return c;
}

Rc<Overlay> Area::spawnOverlay(const std::string& descriptor,
    vicoord coord, const std::string& phase)
{
    auto o = Rc<Overlay>(new Overlay);
    if (!o->init(descriptor, phase)) {
        // Error logged.
        return Rc<Overlay>();
    }
    o->setArea(this);
    o->teleport(coord);
    insert(o);
    return o;
}

void Area::insert(Rc<Character> c)
{
    characters.insert(c);
}

void Area::insert(Rc<Overlay> o)
{
    overlays.insert(o);
}


vicoord Area::phys2virt_vi(icoord phys) const
{
    return grid.phys2virt_vi(phys);
}

rcoord Area::phys2virt_r(icoord phys) const
{
    return grid.phys2virt_r(phys);
}

icoord Area::virt2phys(vicoord virt) const
{
    return grid.virt2phys(virt);
}

icoord Area::virt2phys(rcoord virt) const
{
    return grid.virt2phys(virt);
}

rcoord Area::virt2virt(vicoord virt) const
{
    return grid.virt2virt(virt);
}

vicoord Area::virt2virt(rcoord virt) const
{
    return grid.virt2virt(virt);
}

DataArea* Area::getDataArea()
{
    return dataArea;
}

int Area::depthIndex(double depth) const
{
    return grid.depthIndex(depth);
}

double Area::indexDepth(int idx) const
{
    return grid.indexDepth(idx);
}



static void drawTile(Tile& tile, int x, int y, double depth, int tileDimY)
{
    TileType* type = (TileType*)tile.parent;
    if (type) {
        time_t now = World::instance().time();
        Image* img = type->anim.frame(now);
        if (img) {
            rvec2 drawPos(
                    double(x * (int)img->width()),
                    double(y * (int)img->height())
            );
            double zOffset = drawPos.y / tileDimY * ISOMETRIC_ZOFF_PER_TILE;
            img->draw(drawPos.x, drawPos.y, depth + zOffset);
        }
    }
}

void Area::drawTiles()
{
    icube tiles = visibleTiles();
    for (int z = tiles.z1; z < tiles.z2; z++) {
        assert_(0 <= z && z <= grid.dim.z);
        double depth = grid.idx2depth[(size_t)z];
        for (int y = tiles.y1; y < tiles.y2; y++) {
            for (int x = tiles.x1; x < tiles.x2; x++) {
                Tile* tile = getTile(x, y, z);
                // We are certain the Tile exists.
                drawTile(*tile, x, y, depth, grid.tileDim.y);
            }
        }
    }
}

void Area::drawEntities()
{
    for (auto& character : characters) {
        character->draw();
    }
    for (auto& overlay : overlays) {
        overlay->draw();
    }
    player->draw();
}
