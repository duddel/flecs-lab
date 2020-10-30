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
    // this system is expressed with a signature string (components)
    // the columns (Position) are retrieved from the iterator manually
    ecs.system<>(nullptr, "[out] Position, Asteroid")
        .iter([](flecs::iter &it) {
            flecs::column<Position> colPos(it, 1);
            for (auto row : it)
            {
                printf("Asteroid here: (%i,%i)\n", colPos[row].x, colPos[row].y);
            }
        });

    // system: move, print Rockets
    // this system is expressed with components as template parameters
    ecs.system<Position, Rocket>()
        .iter([](flecs::iter &it, Position *p, Rocket *r) {
            for (auto row : it)
            {
                p[row].y++;
                printf("Rocket moved: (%i,%i)\n", p[row].x, p[row].y);
            }
        });

    // system: match everything that has a Position
    auto sysPositionIter = // keep the system handle to run the system manually (below)
        ecs.system<const Position>()
            // the default "kind" of a system is the pipeline phase flecs::OnUpdate. setting kind = 0
            // means not assigning the system to any phase. it can only be run manually (via run())
            .kind(0)
            .iter([](flecs::iter &it, const Position *p) {
                // iter()-systems are invoked multiple times, for all "types" (list of components) that appear.
                // here: invoked for type (Position, Asteroid) and for type (Position, Rocket)
                printf("iter() invoked for Position, %i entitie(s):\n", it.count());
                for (auto row : it)
                {
                    flecs::type eT = it.entity(row).type();
                    // p can be accessed as a c-array. components for each type are algined continuously on memory.
                    printf("  %s here: (%i,%i) (matched via iter())\n", eT.str().c_str(), p[row].x, p[row].y);
                }
            });

    // system: match everything that has a Position, via each()
    ecs.system<const Position>("sysPositionEach") // assign a name to the system to retrieve it later from world
        .kind(0)                                  // see system above
        .each([](flecs::entity e, const Position &p) {
            flecs::type eT = e.type();
            printf("%s here: (%i,%i) (matched via each())\n", eT.str().c_str(), p.x, p.y);
            // todo: show what else can be done with an entity:
            // entities can be asked if they have a component, like so:
            //   if(e.has<Asteroid>()){...}
        });

    // system: collide Rockets with Asteroids
    // todo: is it safe to capture variables here? qryAsteroid is captured
    // todo: how to get qryAsteroid out of the world, via it.world()?
    ecs.system<>(nullptr, "Position,Rocket")
        .iter([&](flecs::iter &it) {
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

    // these systems have been declared with kind(0), which means they
    // are not part of any pipeline phase and can only be run manually
    printf("------- calling sysPositionIter -------\n");
    sysPositionIter.run();
    printf("------- calling sysPositionEach -------\n");
    ecs.system("sysPositionEach").run();

    printf("------- starting simulation -------\n");
    // progress until no Rockets left
    while (ecs.count<Rocket>() > 0)
    {
        // concerning invocation order:
        // with system::kind(), the pipeline phase (default flecs::OnUpdate), a system runs in,
        // can be set during system declaration. the order of system invocation per
        // pipeline phase is, by default, their declaration order
        ecs.progress();
        printf("------- end of frame -------\n");
    }

    return 0;
}
