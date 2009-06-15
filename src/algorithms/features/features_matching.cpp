
// Features detection and (noisy) matching

// ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
// Headers

#include "features_matching.hpp"

#include "features_detection.hpp" // for the definition of FASTFeature

// implementation specific headers
#include <iostream> // cout definition
#include <limits> // to use numeric_limits<float>::max() and similars


namespace uniclop
{
namespace algorithms
{
namespace features
{

using namespace std;

// ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
// Class FASTFeaturesMatcher methods implementations


args::options_description FASTFeaturesMatcher::get_options_description()
{

    args::options_description desc("FASTFeaturesMatcher options");
    desc.add_options()

    /* ( "blur_sigma,b", args::value<double>()->default_value(1.0f),
       "defines a bluring level for images") */

    ;

    return desc;
}

FASTFeaturesMatcher::FASTFeaturesMatcher(args::variables_map &options)
{
    return;
}

FASTFeaturesMatcher::~FASTFeaturesMatcher()
{
    return;
}

vector< ScoredMatch<FASTFeature> >& FASTFeaturesMatcher::match(
    const vector<FASTFeature>& features_list_a,
    const vector<FASTFeature>& features_list_b)
{
    matchings.clear();

    return matchings;
}

// ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
// Class SimpleFeaturesMatcher methods implementations

template<typename T>
args::options_description SimpleFeaturesMatcher<T>::get_options_description()
{

    args::options_description desc("SimpleFeaturesMatcher options");
    desc.add_options()

    ( "simple_features_matcher.max_distance", args::value<float>()->default_value(1e10),
      "maximum sum of squared difference between two features to do a match (16 uint8_t pixels)")

    ( "simple_features_matcher.num_near_features", args::value<int>()->default_value(3),
      "for each feature the nearest num_near_features will be proposed as putative matches")

    ;

    return desc;
}


template<typename T>
SimpleFeaturesMatcher<T>::SimpleFeaturesMatcher(args::variables_map &options)
{

    _max_distance = 40000.0f;
    _num_near_features = 5;

    if ( options.count("simple_features_matcher.max_distance") )
    {
        _max_distance = options["simple_features_matcher.max_distance"].as<float>();
    }

    if ( options.count("simple_features_matcher.num_near_features") )
    {
        _num_near_features = options["simple_features_matcher.num_near_features"].as<int>();
    }

    return;
}


template<typename T>
SimpleFeaturesMatcher<T>::~SimpleFeaturesMatcher()
{
    return;
}


template<typename T>
vector< ScoredMatch<T> >& SimpleFeaturesMatcher<T>::match(
    const vector<T>& features_list_a,
    const vector<T>& features_list_b)
{

    matchings.clear();

    const float &max_distance = _max_distance;
    const int &num_near_features = _num_near_features;

    typename vector<T>::const_iterator features_it_a, features_it_b;

    for (features_it_a = features_list_a.begin();
            features_it_a != features_list_a.end();
            ++features_it_a)
    { // for each feature in list a

        // search for the num_near_features nearest features

        vector< ScoredMatch<T> > candidate_matches(num_near_features);
        // keep the list of candidates for the current feature

        typename vector< ScoredMatch<T> >::iterator candidate_matches_it;

        typename vector< ScoredMatch<T> >::iterator worst_candidate_it = candidate_matches.begin();

        for (features_it_b = features_list_b.begin();
                features_it_b != features_list_b.end();
                ++features_it_b)
        { // for each feature in list b

            float t_distance = features_it_a->distance( *features_it_b );

            if (t_distance > max_distance) break;
            // distance is out of the range of interest, so we skip this one

            // we suppose that num_near_features is small so a direct iteration
            // over the candidates list is the fastest option
            for (candidate_matches_it = candidate_matches.begin();
                    candidate_matches_it != candidate_matches.end();
                    ++candidate_matches_it)
            { // search for the worst candidate

                if ( candidate_matches_it->feature_a != &(*features_it_a) )
                {
                    // candidate slot has not been initialized
                    // by default this is the worst one
                    worst_candidate_it = candidate_matches_it;

                    // we set some members (the others will be done later)
                    worst_candidate_it->feature_a = &(*features_it_a);
                    worst_candidate_it->distance = numeric_limits<float>::max();

                    break; // stop searching for worst
                }
                else
                { // if candidate has been initialized
                    if ( (worst_candidate_it->distance) < (candidate_matches_it->distance) )
                    { // this one is the worst found up to now
                        worst_candidate_it = candidate_matches_it;
                    }
                }
            } // end of 'search for the worst candidate'

            if ( (worst_candidate_it->distance) > t_distance)
            { // current candidate is better than the worst of the previous ones
                // thus we replace it
                worst_candidate_it->feature_b = &(*features_it_b);
                worst_candidate_it->distance = t_distance;
            }

        } // end of 'for each feature in list b'


        typename vector< ScoredMatch<T> >::const_iterator const_candidate_matches_it;

        // now candidate_matches has the list of putative matches for feature_a
        for (const_candidate_matches_it = candidate_matches.begin();
                const_candidate_matches_it != candidate_matches.end();
                ++const_candidate_matches_it)
        { // add the putative matches to the result list

            if ( const_candidate_matches_it->feature_a == &(*features_it_a) ) // sanity check
            {
                matchings.push_back(*const_candidate_matches_it); // copy the content
            }
        }

    } // end of 'for each feature in list a'

    return matchings;
}


// ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
// Force the compilation of the following types
// for some strange reason, in linux, this hast to be at the end of the defitions (?!)
class FASTFeature; // forward declaration
template class SimpleFeaturesMatcher<FASTFeature>;

//class SIFTFeature; // forward declaration
//template class SimpleFeaturesMatcher<SIFTFeature>;
// ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=

}
}
}