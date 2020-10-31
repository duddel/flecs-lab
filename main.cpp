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

    // query for Asteroids
    auto qryAsteroid = ecs.query<Position, Asteroid>();

    // filter for Asteroids
    flecs::filter filtAsteroid =
        flecs::filter(ecs)
            .include<Position>()
            .include<Asteroid>()
            .include_kind(flecs::MatchAll);

    // system: collide Rockets with Asteroids
    // todo: is it safe to capture variables here? qryAsteroid/filtAsteroid are captured
    // todo: can we get qryAsteroid/filtAsteroid from it.world()?
    ecs.system<>(nullptr, "Position,Rocket")
        .iter([&](flecs::iter &it) {
            flecs::column<Position> p(it, 1);
            // loop over all rows and get Position (of a Rocket) from column p.
            // in a nested loop, use a query or a filter to compare Rocket
            // Position to Position of all Asteroids in the world.
            for (auto row : it)
            {
#if 1 // query/filter switch
                for (auto it1 : qryAsteroid)
#else
                for (auto it1 : it.world().filter(filtAsteroid))
#endif
                {
                    auto p1 = it1.table_column<Position>();
                    for (auto row1 : it1)
                    {
                        if (p[row].x == p1[row1].x && p[row].y == p1[row1].y)
                        {
                            printf("BOOM: (%i,%i)\n", p1[row1].x, p1[row1].y);
                            it1.entity(row1).destruct(); // kill Asteroid
                            it.entity(row).destruct();   // kill Rocket
                        }
                    }
                }
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

    // use a query or a filter to get all entities, that have a Position.
    // this is essentially doing what a system does, without having a function
    // or lambda to be called (each(), iter()), but rather iteration over tables
    // and rows by oneself.
    // this is used in the system "collide Rockets with Asteroids" above, to implement
    // a nested search for Asteroids inside the system.
    // queries are faster to iterate, but slower to construct, than filters.
    // queries can also be used with each() and iter(), like systems.
#if 1 // query/filter switch
    printf("------- querying Position -------\n");
    auto qryPosition = ecs.query<Position>();

    for (auto it : qryPosition)
#else
    printf("------- filtering Position -------\n");
    flecs::filter filtPosition = flecs::filter(ecs).include<Position>().include_kind(flecs::MatchAll);

    for (auto it : ecs.filter(filtPosition))
#endif
    {
        printf("query/filter for Position, iterating over %i rows:\n", it.count());
        auto p = it.table_column<Position>();
        for (auto row : it)
        {
            flecs::type eT = it.entity(row).type();
            printf("  %s here: (%i,%i) (query/filter)\n", eT.str().c_str(), p[row].x, p[row].y);
        }
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
