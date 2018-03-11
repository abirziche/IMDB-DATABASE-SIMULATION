/* Copyright 2017 Andrei Raduta, Andrei Birziche */

#include <string>
#include <vector>

#include "include/imdb.h"

Movie::Movie(std::string movie_name, int timestamp, std::vector<std::string>
    categories, std::string director_name, std::vector<std::string> actor_ids) {

    movieName.assign(movie_name);
    aparitionTime = timestamp;
    categoryList = categories;
    directorName.assign(director_name);
    actorList = actor_ids;
    ratingSum = 0;
}

std::string timestampToYear(int year) {
    time_t t = year;
    struct tm newTime;
    gmtime_r(&t, &newTime);
    return std::to_string(1900 + newTime.tm_year);
}
