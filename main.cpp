/*
Copyright (c) 2020 Alexander Scholz

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include <cstdio> // printf()
#include "flecs.h"

// components
struct Position
{
   int x;
   int y;
};

// tags ("empty" components)
struct Asteroid
{
};

struct Rocket
{
};

int main()
{
   flecs::world ecs;

   // register components (and tags)
   ecs.component<Position>();
   ecs.component<Asteroid>();
   ecs.component<Rocket>();

   // query for Asteroids is evaluated in a system below
   auto qryAsteroid = ecs.query<Position, Asteroid>();

   // system: print Asteroids
   ecs.system<>(nullptr, "Position,Asteroid").iter([](flecs::iter &it) {
      flecs::column<Position> colPos(it, 1);
      for (auto row : it)
      {
         printf("Asteroid here: (%i,%i)\n", colPos[row].x, colPos[row].y);
      }
   });

   // system: move, print Rockets
   ecs.system<>(nullptr, "Position,Rocket").iter([](flecs::iter &it) {
      flecs::column<Position> colPos(it, 1);
      for (auto row : it)
      {
         colPos[row].y++;
         printf("Rocket moved: (%i,%i)\n", colPos[row].x, colPos[row].y);
      }
   });

   // system: collide Rockets with Asteroids
   // todo: is it safe to capture variables here? qryAsteroid is captured
   // todo: how to get qryAsteroid out of the world, via it.world()?
   ecs.system<>(nullptr, "Position,Rocket").iter([&](flecs::iter &it) {
      flecs::column<Position> colPos(it, 1);
      for (auto row : it)
      {
         // todo: is it safe to capture colPos, row, it here?
         qryAsteroid.each([&](flecs::entity e, Position &p, Asteroid &a) {
            if (colPos[row].x == p.x && colPos[row].y == p.y)
            {
               printf("BOOM: (%i,%i)\n", p.x, p.y);
               // todo: is it appropriate to destruct entities here, this way?
               e.destruct();              // kill Asteroid
               it.entity(row).destruct(); // kill Rocket
            }
         });
      }
   });

   // add entities, Rockets will move up until collision
   //       A
   //    A
   // A
   // R  R  R
   for (int i = 0; i < 3; i++)
   {
      ecs.entity().set<Position>({i, i + 1}).add<Asteroid>();
      ecs.entity().set<Position>({i, 0}).add<Rocket>();
   }

   for (int i = 0; i < 4; i++)
   {
      ecs.progress();
      printf("------- end of frame -------\n");
   }

   return 0;
}
