/* Copyright 2017 Andrei Raduta, Andrei Birziche */

#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <ctime>
#include <set>

#include "include/imdb.h"

IMDb::IMDb() {
}

IMDb::~IMDb() {
}

void IMDb::add_movie(std::string movie_name,
                     std::string movie_id,
                     int timeS,  // unix timestamp when movie was launched
                     std::vector<std::string> categories,
                     std::string director_name,
                     std::vector<std::string> actor_ids) {
        // Adaug in hashtable-ul cu filme noul film cu toate atributele lui
        mapMovies.emplace(movie_id, Movie(movie_name, timeS,
                        categories, director_name, actor_ids));

        // Pentru 2nd degree
        for ( auto it1 = actor_ids.begin(); it1 != actor_ids.end(); it1++ ) {
            for ( auto it2 = actor_ids.begin();
                                            it2 != actor_ids.end(); it2++ ) {
                if ( mapActors.at(*it1).Colegi.find(*it2) ==
                                          mapActors.at(*it1).Colegi.end() ) {
                      if ( it1 != it2 ) {
                          mapActors.at(*it1).Colegi.emplace(*it2, 1);
                      }
                } else {
                    mapActors.at(*it1).Colegi.at(*it2)++;
                }
            }
        }

        mapMovies.emplace(movie_id, Movie(movie_name, timeS,
                        categories, director_name, actor_ids));

        // Verific in map-ul directorilor daca acesta exista deja
        if ( mapDirectors.find(director_name) == mapDirectors.end() ) {
            std::unordered_set<std::string> actors;
            mapDirectors.emplace(director_name, actors);
        }

        // Verific daca data aparitiei este mai mica sau mai mare pentru
        // fiecare actor care joaca in acest film
        for ( unsigned itActor = 0; itActor < actor_ids.size(); itActor++ ) {
            if ( mapActors.at(actor_ids[itActor]).firstAparition > timeS ) {
                mapActors.at(actor_ids[itActor]).firstAparition = timeS;
            }

            if ( mapActors.at(actor_ids[itActor]).lastAparition < timeS ) {
                mapActors.at(actor_ids[itActor]).lastAparition = timeS;
            }

            // Adaug id-urile actorilor in map-ul directorilor
            mapDirectors.at(director_name).emplace(actor_ids[itActor]);
        }

        // Adaug filmul si anul intr-un hashtable special
        timeToMovie.emplace(timeS, movie_id);

        // Adaug filmul in vectorul de filme populare
		TopkPM.push_back(PopularMovies(movie_id));
		pplmov = false;
}

void IMDb::add_user(std::string user_id, std::string name) {
    mapUsers.emplace(user_id, User(name));
}

void IMDb::add_actor(std::string actor_id, std::string name) {
    mapActors.emplace(actor_id, Actor(name));
}

void IMDb::add_rating(std::string user_id, std::string movie_id, int rating) {
    mapMovies.at(movie_id).ratingList.emplace(user_id, rating);

    // Adaug si acest rating in suma ratingurilor pentru acest film
    mapMovies.at(movie_id).ratingSum += rating;

    // Pentru most populare
    for ( uint i = 0; i < TopkPM.size(); i++ ) {
        if ( TopkPM[i].movie_id.compare(movie_id) == 0 ) {
            TopkPM[i].nrRating++;
            pplmov = false;
            break;
        }
    }
}

void IMDb::update_rating(std::string user_id, std::string movie_id, int rtg) {
    // Scad valoarea veche din suma rating-urilor si o adaug pe cea noua
    mapMovies.at(movie_id).ratingSum -=
                                mapMovies.at(movie_id).ratingList.at(user_id);
    mapMovies.at(movie_id).ratingSum += rtg;

    // Modific valoarea din lista a ratingului acestui user
    mapMovies.at(movie_id).ratingList.at(user_id) = rtg;
}

void IMDb::remove_rating(std::string user_id, std::string movie_id) {
    // Updatez media rating-urilor
    mapMovies.at(movie_id).ratingSum -=
                                mapMovies.at(movie_id).ratingList.at(user_id);
    mapMovies.at(movie_id).ratingList.erase(user_id);

    // Pentru most populare
    for ( uint i = 0; i < TopkPM.size(); i++ ) {
        if ( TopkPM[i].movie_id.compare(movie_id) == 0 ) {
            TopkPM[i].nrRating--;
            pplmov = false;
            break;
        }
    }
}

// Done
std::string IMDb::get_rating(std::string movie_id) {
    double avgRating = (double) mapMovies.at(movie_id).ratingSum;

    // Verific daca media este 0 == nu s-a dat inca niciun rating
    if ( mapMovies.at(movie_id).ratingList.size() == 0 ) {
        return "none";
    }

    // Formatarea rezultatului
    avgRating /= mapMovies.at(movie_id).ratingList.size();
    avgRating = roundf(avgRating * 100) / 100;
    if (avgRating == 10) {
        return std::to_string(avgRating).substr(0, 5);
    }
    return std::to_string(avgRating).substr(0, 4);
}

// Done
std::string IMDb::get_longest_career_actor() {
    int careerDuration = -1;
    std::string actor;
    bool ok = true;

    // Calculez pentru fiecare actor durata carierei si pastrez minimul
    // mai intai dupa durata, apoi dupa id, prin < ci nu <=
    for ( auto itAc = mapActors.begin(); itAc != mapActors.end(); itAc++ ) {
        int dif = itAc->second.lastAparition - itAc->second.firstAparition;

        if ( careerDuration < dif ) {
            careerDuration = dif;
            actor.assign(itAc->first);
            ok = false;
        }
    }

    // Daca nu s-a gasit niciun actor care sa fi jucat in vreun film
    if ( ok ) {
        return "none";
    }
    return actor;
}

// Done
std::string IMDb::get_most_influential_director() {
    int max = 0;
    std::string maxDirector;

    for ( auto itDir = mapDirectors.begin();
               itDir != mapDirectors.end(); itDir++ ) {
        if ( max < static_cast<int>(itDir->second.size()) ) {
            max = itDir->second.size();
            maxDirector.assign(itDir->first);
        }
    }
    return maxDirector;
}

// Done
std::string IMDb::get_best_year_for_category(std::string category) {
    bool ok = false;  // Categoria nu exista
    double t;
    std::string y;

    class Years {
     public:
    	double sum;
    	int nrfilme;
    	Years() {
    		sum = 0;
    		nrfilme = 0;
    	}
    };

    std::map<std::string, Years> Category;
    for ( auto it = mapMovies.begin(); it != mapMovies.end(); it++ ) {
        for ( uint i = 0; i < it->second.categoryList.size(); i++ )  {
            if ( it->second.categoryList[i] == category ) {
                y = timestampToYear(it->second.aparitionTime);

                // Daca gasim anul in map
                Category.emplace(y, Years());

                // Daca exista ratinguri
                if ( it->second.ratingList.size() != 0 ) {
                    ok = 1;
                    t = static_cast<double>(it->second.ratingSum);
                    t /= it->second.ratingList.size();
                    Category.at(y).sum += t;
                    Category.at(y).nrfilme++;
                }
            }
        }
    }

	double max = -1;
	for ( auto it = Category.begin(); it != Category.end(); it++ ) {
		t = static_cast<double>(it->second.sum) / it->second.nrfilme;
		if ( t > max ) {
			max = t;
			y = it->first;
		}
	}

	if ( ok == 0 ) {
		return "none";
    } else {
        return y;
    }
}

// Done
std::string IMDb::get_2nd_degree_colleagues(std::string actor_id) {
    std::string toReturn;
    std::set<std::string> Colegi2;

    for ( auto it1 = mapActors.at(actor_id).Colegi.begin();
    	                   it1 != mapActors.at(actor_id).Colegi.end(); it1++ ) {
        for ( auto it2 = mapActors.at(it1->first).Colegi.begin();
        	             it2 != mapActors.at(it1->first).Colegi.end(); it2++ ) {
            if ( mapActors.at(actor_id).Colegi.find(it2->first) ==
               mapActors.at(actor_id).Colegi.end() && it2->first != actor_id ) {
                    Colegi2.emplace(it2->first);
            }
        }
    }

    if ( Colegi2.size() == 0 ) {
        return "none";
    }

    for ( auto it1 = Colegi2.begin(); it1 != Colegi2.end(); it1++ ) {
        toReturn += *it1;
        toReturn += ' ';
    }
    return toReturn.substr(0, toReturn.size() - 1);
}

// Done
std::string IMDb::get_top_k_most_recent_movies(int k) {
    if ( mapMovies.size() == 0 ) {
        return "none";
    }

    std::string toReturn;
    for ( auto it = timeToMovie.crbegin();
                            it != timeToMovie.crend() && k != 0; it++, k-- ) {
    	toReturn += it->second;
    	toReturn += ' ';
    }
    return toReturn.substr(0, toReturn.size() - 1);
}

// Done
std::string IMDb::get_top_k_actor_pairs(int k) {
    std::vector<Pair> myVec;

    // Parcurg toti actorii si trebuie sa vad cu cine au jucat cel mai mult
    for ( auto it1 = mapActors.begin(); it1 != mapActors.end(); it1++ ) {
        for ( auto it2 = mapActors.at(it1->first).Colegi.begin();
                        it2 != mapActors.at(it1->first).Colegi.end(); it2++ ) {
            // Verific daca am deja perechea asta de actori in vector
            bool found = false;

            for ( uint i = 0; i < myVec.size(); i++ ) {
                if ( myVec[i].actorOne.compare(it1->first) == 0 &&
                     myVec[i].actorTwo.compare(it2->first) == 0 ) {
                    found = true;
                    break;
                }
                if ( myVec[i].actorTwo.compare(it1->first) == 0 &&
                     myVec[i].actorOne.compare(it2->first) == 0 ) {
                    found = true;
                    break;
                }
            }

            // Daca nu s-a gasit
            if ( !found ) {
                myVec.push_back(Pair(it1->first, it2->first, it2->second));
            }
        }
    }

    if ( myVec.size() == 0 ) {
        return "none";
    }

    std::sort(myVec.begin(), myVec.end());
    std::string toReturn = "";
    if ( static_cast<uint>(k) > myVec.size() ) {
        k = myVec.size();
    }

    for ( int i = 0; i < k; i++ ) {
        toReturn += "(" + myVec[i].actorOne + " " + myVec[i].actorTwo + " ";
        toReturn += std::to_string(myVec[i].moviesTogh) + ") ";
    }
    return toReturn.substr(0, toReturn.size() - 1);
}

// Done
std::string IMDb::get_top_k_partners_for_actor(int k, std::string actor_id) {
    std::vector<Pair> myVec;

    // Parcurg toti actorii pereche cu cel de la parametru
    for ( auto it1 = mapActors.at(actor_id).Colegi.begin();
                    it1 != mapActors.at(actor_id).Colegi.end(); it1++ ) {
        // Verific daca am deja perechea asta de actori in vector
        bool found = false;

        for ( uint i = 0; i < myVec.size(); i++ ) {
            if ( myVec[i].actorOne.compare(it1->first) == 0 ) {
                found = true;
                break;
            }
        }

        // Daca nu s-a gasit
        if ( !found ) {
            myVec.push_back(Pair(it1->first, actor_id, it1->second));
        }
    }

    if ( myVec.size() == 0 ) {
        return "none";
    }

    std::sort(myVec.begin(), myVec.end());
    std::string toReturn = "";
    if ( static_cast<uint>(k) > myVec.size() ) {
        k = myVec.size();
    }

    for ( int i = 0; i < k; i++ ) {
        toReturn += myVec[i].actorOne + " ";
    }
    return toReturn.substr(0, toReturn.size() - 1);
}

// Birziche
std::string IMDb::get_top_k_most_popular_movies(int k) {
    if ( mapMovies.size() == 0 ) {
        return "none";
    }

    if ( !pplmov ) {
    	std::sort(TopkPM.begin(), TopkPM.end());
    	pplmov = true;
    }

    std::string toReturn = "";
    for ( uint i = 0; i < TopkPM.size() && k != 0; i++, k-- ) {
    	toReturn += TopkPM[i].movie_id + " ";
    }

    return toReturn.substr(0, toReturn.size() - 1);
}

// Done
std::string IMDb::get_avg_rating_in_range(int start, int end) {
    double avgRating = 0;
    int k = 0;

    // Parcurg filmele si vad daca au vreun rating si sunt in acea perioada
    for ( auto it = mapMovies.begin(); it != mapMovies.end(); it++ ) {
        if ( it->second.ratingList.size() != 0 &&
                                      (start <= it->second.aparitionTime &&
                                         end >= it->second.aparitionTime) ) {
            avgRating += (static_cast<double>(it->second.ratingSum) /
                    static_cast<double>(it->second.ratingList.size()));
            k++;
        }
    }

    if ( k == 0 ) {
        return "none";
    } else {
        avgRating /= k;
        avgRating = roundf(avgRating * 100) / 100;
        if (avgRating == 10) {
            return std::to_string(avgRating).substr(0, 5);
        }
        return std::to_string(avgRating).substr(0, 4);
    }
}
