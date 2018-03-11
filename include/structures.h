/* Copyright 2017 Andrei Raduta, Andrei Birziche */

#ifndef __STRUCTURES__H__
#define __STRUCTURES__H__

#include <iterator>
#include <cmath>
#include <climits>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

struct Actor {
    std::string actorName;
    std::map<std::string, int> Colegi;
    int firstAparition;
    int lastAparition;

    explicit Actor(std::string newName): actorName(newName),
                            firstAparition(INT_MAX), lastAparition(0) {}
};

struct User {
    std::string userName;
    explicit User(std::string newName): userName(newName) {}
};

struct Movie {
    std::string movieName;
    int aparitionTime;
    std::string directorName;
    std::vector<std::string> actorList;
    std::vector<std::string> categoryList;
    std::unordered_map<std::string, int> ratingList;  // User - rating
    int ratingSum;

    Movie(std::string, int, std::vector<std::string>, std::string,
                            std::vector<std::string>);
};

std::string timestampToYear(int year);

struct PopularMovies {
    std::string movie_id;
    int nrRating;
    explicit PopularMovies(std::string id): movie_id(id), nrRating(0) {}

    bool operator < (const PopularMovies& a) const {
        if ( nrRating > a.nrRating ) {
            return true;
        } else if ( nrRating == a.nrRating ) {
            if ( movie_id.compare(a.movie_id) < 0 ) {
                return true;
            }
            return false;
        }
        return false;
    }
};

struct Pair {
    std::string actorOne;
    std::string actorTwo;
    int moviesTogh;
    Pair(std::string new1, std::string new2, int new3) {
        actorOne.assign(new1);
        actorTwo.assign(new2);
        moviesTogh = new3;
    }

    bool operator < (const Pair& other) const {
        if ( moviesTogh > other.moviesTogh ) {
            return true;
        } else if ( moviesTogh == other.moviesTogh ) {
            if ( actorOne.compare(other.actorOne) < 0 ) {
                return true;
            } else if ( actorOne.compare(other.actorOne) == 0 ) {
                if ( actorTwo.compare(other.actorTwo) < 0 ) {
                    return true;
                }
                return false;
            }
        }
        return false;
    }
};

#endif
