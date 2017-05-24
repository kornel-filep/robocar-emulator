/**
 * @brief Justine - this is a rapid prototype for development of Robocar City Emulator
 *
 * @file car.cpp
 * @author  Norbert Bátfai <nbatfai@gmail.com>
 * @version 0.0.10
 *
 * @section LICENSE
 *
 * Copyright (C) 2014 Norbert Bátfai, batfai.norbert@inf.unideb.hu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 * Robocar City Emulator and Robocar World Championship
 *
 * desc
 *
 */

#include <ctime>
#include <car.hpp>
#include <traffic.hpp>
#include <boost/iterator/iterator_concepts.hpp>

#include <boost/random/mersenne_twister.hpp> //Random generator.

// Probability mass function. And discrete probability distribution.
//Will aid us in picking the next node to go to.
#include <boost/random/discrete_distribution.hpp> 

justine::robocar::Car::Car (
  justine::robocar::Traffic & traffic,
  justine::robocar::CarType type ) :  traffic ( traffic ), m_type ( type )
{

}

justine::robocar::AdjacencyList justine::robocar::AntCar::alist;
justine::robocar::AdjacencyList justine::robocar::AntCar::alist_evaporate;

justine::robocar::AntCar::AntCar ( justine::robocar::Traffic & traffic ) : justine::robocar::Car ( traffic )
{

}


justine::robocar::SmartCar::SmartCar ( justine::robocar::Traffic & traffic,
                                       justine::robocar::CarType type,
                                       bool guided ) : justine::robocar::Car ( traffic, type ), m_guided ( guided )
{

}

justine::robocar::CopCar::CopCar ( justine::robocar::Traffic & traffic,
                                   bool guided, const char *name ) :
  justine::robocar::SmartCar ( traffic, CarType::POLICE, guided ), m_name ( name )
{

}

void justine::robocar::Car::init()
{

  m_from = traffic.node();
  m_to = 0;
  m_step = 0;
  traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to ) +1 );

}

void justine::robocar::SmartCar::init()
{

  if ( m_guided )
    {
      /*
            osmium::unsigned_object_id_type ini {2969934868};

            if ( traffic.hasNode ( ini ) )
              {
                m_from = ini;
              }
            else
              {
                m_from = traffic.node();
              }
      */

      m_from = traffic.node();

      m_to = 0;
      m_step = 0;
      traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to ) +1 );

    }
  else
    Car::init();

}

osmium::unsigned_object_id_type justine::robocar::AntCar::ant ( void )
{

  AdjacencyList::iterator iter= AntCar::alist.find ( m_from );

  WayNodesVect::iterator i = std::max_element ( iter->second.first.begin(), iter->second.first.end() );

  osmium::unsigned_object_id_type next_m_to = std::distance ( iter->second.first.begin(), i );

  ++*i;

  return next_m_to;
}

osmium::unsigned_object_id_type justine::robocar::AntCar::ant_rnd ( void )
{

  AdjacencyList::iterator iter= AntCar::alist.find ( m_from );

  int sum = std::accumulate ( iter->second.first.begin(), iter->second.first.end(), 0 );

  int rnd = std::rand() % sum;

  int sum2 = 0;

  WayNodesVect::iterator j=iter->second.first.begin();
  for ( ; j!= iter->second.first.end(); ++j )
    {
      sum2 += *j;
      if ( sum2 >= rnd )
        break;
    }

  osmium::unsigned_object_id_type next_m_to = std::distance ( iter->second.first.begin(), j );

  ++*j;

  return next_m_to;
}

osmium::unsigned_object_id_type justine::robocar::AntCar::ant_rernd ( void )
{

  AdjacencyList::iterator iter= AntCar::alist.find ( m_from );

  WayNodesVect cpv = iter->second.first;

  int sum = std::accumulate ( cpv.begin(), cpv.end(), 0 );

  int res = ( int ) ( ( double ) sum/.75 );

  int total = sum + cpv.size() * ( res/cpv.size() );

  for ( osmium::unsigned_object_id_type& v : cpv )
    v += res/cpv.size();

  int rnd = std::rand() % total;

  int sum2 = 0;

  WayNodesVect::iterator j=cpv.begin();
  for ( ; j!= cpv.end(); ++j )
    {
      sum2 += *j;
      if ( sum2 >= rnd )
        break;
    }

  osmium::unsigned_object_id_type next_m_to = std::distance ( cpv.begin(), j );

  ++*j;

  return next_m_to;
}

osmium::unsigned_object_id_type justine::robocar::AntCar::ant_mrernd ( void )
{
  AdjacencyList::iterator iter = AntCar::alist.find ( m_from );
  AdjacencyList::iterator iter2 = AntCar::alist_evaporate.find ( m_from );

  for ( WayNodesVect::iterator j=iter->second.first.begin(); j!= iter->second.first.end(); ++j )
    {

      int del = traffic.get_time() - iter2->second.first[std::distance ( iter->second.first.begin(), j )];

      int pheromone = *j - del;

      if ( pheromone > 1 ) *j = pheromone;
      else *j = 1;
    }

  WayNodesVect cpv = iter->second.first;

  int sum = std::accumulate ( cpv.begin(), cpv.end(), 0 );

  int res = ( int ) ( ( double ) sum/.6 );

  int total = sum + cpv.size() * ( res/cpv.size() );

  for ( osmium::unsigned_object_id_type& v : cpv )
    v += res/cpv.size();

  int rnd = std::rand() % total;

  int sum2 = 0;

  WayNodesVect::iterator j=cpv.begin();
  for ( ; j!= cpv.end(); ++j )
    {
      sum2 += *j;
      if ( sum2 >= rnd )
        break;
    }

  osmium::unsigned_object_id_type next_m_to = std::distance ( cpv.begin(), j );

  ++*j;
  iter2->second.first[std::distance (cpv.begin(), j )] = traffic.get_time();

  return next_m_to;
}


void justine::robocar::AntCar::nextSmarterEdge ( void )
{
  osmium::unsigned_object_id_type next_m_from = traffic.alist ( m_from, m_to );
  size_t nes = traffic.nedges ( next_m_from );
  if ( !nes )
    return;

  osmium::unsigned_object_id_type next_m_to;

  if ( traffic.get_type() == TrafficType::ANT )
    {
      next_m_to = ant();
    }
  else if ( traffic.get_type() == TrafficType::ANT_RND )
    {
      next_m_to = ant_rnd();
    }
  else if ( traffic.get_type() == TrafficType::ANT_RERND )
    {
      next_m_to = ant_rernd();
    }
  else
    {
      next_m_to = ant_mrernd();
    }


  if ( traffic.alist ( next_m_from, next_m_to ) == m_from )
    next_m_to = ( next_m_to + 1 ) % nes;

  if ( traffic.palist ( next_m_from, next_m_to ) >
       traffic.salist ( next_m_from, next_m_to ) )
    {

      traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to )-1 );

      m_from = next_m_from;
      m_to = next_m_to;
      m_step = 0;

      traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to ) +1 );
    }
}


void justine::robocar::Car::nextSmarterEdge ( void )
{
  osmium::unsigned_object_id_type next_m_from = traffic.alist ( m_from, m_to );
  size_t nes = traffic.nedges ( next_m_from );
  if ( !nes )
    return;
  //rweriting there to include picking by probabilities.
  //osmium::unsigned_object_id_type next_m_to = std::rand() % nes;
  d_vector probv = traffic.getPVector(next_m_from);


  boost::random::mt19937 rgen; //our twister randomgen.
  boost::random::discrete_distribution<> dist(probv);
  rgen.seed(static_cast<unsigned int>(std::time(0))); //Seeding our rgen with time.
  osmium::unsigned_object_id_type next_m_to = dist(rgen); // Chooses based on the distribution of values randomly generated by rgen.
  
  //So for a 2 way node it will output 0.5 0.5, but it will choose according to the weights.
  // Didnt figure out how to give random weights.

#ifdef DEBUG
  std::cout << "Node Currently: " << next_m_from << "\n";

  for (int i = 0; i < nes; i++) std::cout << "Next node probabilities: " << probv.at(i) << "\n";
  std::cout << "dist: " << dist << "\n";
  std::cout << "The node it chose: " << next_m_to << std::endl;  

#endif



  if ( traffic.alist ( next_m_from, next_m_to ) == m_from )
    next_m_to = ( next_m_to + 1 ) % nes;

  if ( traffic.palist ( next_m_from, next_m_to ) >
       traffic.salist ( next_m_from, next_m_to ) )
    {

      traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to )-1 );

      m_from = next_m_from;
      m_to = next_m_to;
      m_step = 0;

      traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to ) +1 );
    }
}

void justine::robocar::Car::nextEdge ( void )
{
  osmium::unsigned_object_id_type next_m_from = traffic.alist ( m_from, m_to );
  size_t nes = traffic.nedges ( next_m_from );
  if ( !nes )
    return;
  osmium::unsigned_object_id_type next_m_to = std::rand() % nes;

  if ( traffic.palist ( next_m_from, next_m_to ) >
       traffic.salist ( next_m_from, next_m_to ) )
    {

      traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to )-1 );

      m_from = next_m_from;
      m_to = next_m_to;
      m_step = 0;

      traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to ) +1 );
    }
}

void justine::robocar::Car::step()
{

  if ( traffic.hasNode ( to_node() ) )
    {

      if ( m_step >= traffic.palist ( m_from, m_to ) )
        {

          nextSmarterEdge();

        }
      else
        ++m_step;


    }
  else
    {

      // car stopped

    }
}

void justine::robocar::SmartCar::nextEdge ( void )
{

  if ( traffic.hasNode ( to_node() ) )
    {

      if ( m_step >= traffic.palist ( m_from, m_to ) )
        {

          osmium::unsigned_object_id_type next_m_from
            = traffic.alist ( m_from, m_to );
          osmium::unsigned_object_id_type next_m_to
            = traffic.naive_node_for_nearest_gangster ( m_from, m_to, m_step );

          if ( traffic.palist ( next_m_from, next_m_to ) >
               traffic.salist ( next_m_from, next_m_to ) )
            {

              traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to )-1 );

              m_from = next_m_from;
              m_to = next_m_to;
              m_step = 0;

              traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to ) +1 );
            }

        }
      else
        ++m_step;

    }
  else
    {

      // car stopped

    }

}

void justine::robocar::SmartCar::nextGuidedEdge ( void )
{
  if ( traffic.hasNode ( to_node() ) )
    {

      if ( m_step >= traffic.palist ( m_from, m_to ) )
        {

          std::vector<unsigned int>::iterator i = std::find ( route.begin(), route.end(), to_node() );

          if ( i == route.end() )
            return;

          osmium::unsigned_object_id_type next_m_to;
          osmium::unsigned_object_id_type next_m_from;

          if ( std::distance ( route.begin(), i ) == route.size() )
            {

              next_m_to = 0;
              next_m_from = to_node();

            }
          else
            {

              next_m_to;
              osmium::unsigned_object_id_type inv = traffic.alist_inv ( to_node(), * ( i+1 ) );

              if ( inv != -1 )
                {
                  next_m_to = inv;
                }
              else
                {
                  return;
                }

              next_m_from = to_node();
            }

          if ( traffic.palist ( next_m_from, next_m_to ) >=
               traffic.salist ( next_m_from, next_m_to ) )
            {

              traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to )-1 );

              m_from = next_m_from;
              m_to = next_m_to;
              m_step = 0;

              traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to ) +1 );
            }
          else
            {
              ;

            }

        }
      else
        ++m_step;

    }
  else
    {

      // car stopped

    }

}


void justine::robocar::SmartCar::step()
{

  if ( m_type == CarType::POLICE )
    {

      if ( m_guided )
        nextGuidedEdge();
      else
        nextEdge();

    }
  else if ( m_type == CarType::CAUGHT )
    {

      return;

    }
  else
    Car::step();

}

osmium::unsigned_object_id_type justine::robocar::Car::to_node() const
{
  return traffic.alist ( m_from, m_to );
}

osmium::unsigned_object_id_type justine::robocar::Car::get_max_steps() const
{
  return traffic.palist ( m_from, m_to );
}

bool justine::robocar::SmartCar::set_route ( std::vector<unsigned int> & route )
{

  if ( route.size() < 2 )
    return false;

  this->route = route;
  m_routed = true;

  if ( m_from == route[0] )
    {


      int next_m_to = traffic.alist_inv ( m_from, route[1] );

      if ( next_m_to != m_to )
        {
          m_to = next_m_to;
          m_step = 0;
        }
    }

  return true;

}

bool justine::robocar::SmartCar::set_fromto ( unsigned int from, unsigned int to )
{
  if ( m_from == from && traffic.hasNode ( to ) )
    {

      traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to ) -1 );

      m_to = traffic.alist_inv ( m_from, to );

      traffic.set_salist ( m_from, m_to, traffic.salist ( m_from, m_to ) +1 );

      return true;
    }

  return false;
}
