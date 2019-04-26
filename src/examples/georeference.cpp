
#include <fstream>

/*
 *  Copyright 2019 © Centre Interdisciplinaire de développement en Cartographie des Océans (CIDCO), Tous droits réservés
 */
#ifndef GEOREFERENCE_CPP
#define GEOREFERENCE_CPP

#include "../datagrams/kongsberg/KongsbergParser.hpp"
#include "../datagrams/xtf/XtfParser.hpp"
#include "../datagrams/s7k/S7kParser.hpp"
#include <iostream>
#include <Eigen/Dense>
#include <string>
#include "../utils/StringUtils.hpp"
#include "../Ping.hpp"
#include "../Attitude.hpp"
#include "../math/Interpolation.hpp"
#include "../Georeferencing.hpp"
#include "../SoundVelocityProfileFactory.hpp"
 #include "../datagrams/kongsberg/KongsbergParser.hpp"
#include "../datagrams/xtf/XtfParser.hpp"
#include "../datagrams/s7k/S7kParser.hpp"
#include <iostream>
#include <string>
#include "../utils/StringUtils.hpp"
using namespace std;

/**Write the information about the georeference*/
void printUsage(){
	std::cerr << "\n\
  NAME\n\n\
     georeference - Produit un nuage de points d'un fichier de datagrammes multifaisceaux\n\n\
  SYNOPSIS\n \
	   [-x lever_arm_x] [-y lever_arm_y] [-z lever_arm_z]\n\
           georeference fichier\n\n\
  DESCRIPTION\n\n \
  Copyright 2017-2019 © Centre Interdisciplinaire de développement en Cartographie des Océans (CIDCO), Tous droits réservés" << std::endl;
	exit(1);
}

/*!
 * \brief Datagramer Georeferencer class extention of the Datagram Processor class
 */
class DatagramGeoreferencer : public DatagramProcessor{
	public:
                /**Create a datagram georeferencer*/
		DatagramGeoreferencer(){

		}

                /**Destroy the datagram georeferencer*/
		~DatagramGeoreferencer(){

		}

                /**
                 * Add the information of a attitude in the vector attitudes
                 * 
                 * @param microEpoch the attitude timestamp
                 * @param heading the attitude heading
                 * @param pitch the attitude pitch
                 * @param roll the attitude roll
                 */
                void processAttitude(uint64_t microEpoch,double heading,double pitch,double roll){
			attitudes.push_back(Attitude(microEpoch,roll,pitch,heading));
                };

                /**
                 * Add the information of a position in the vector positions
                 * 
                 * @param microEpoch the position timestamp
                 * @param longitude the position longitude
                 * @param latitude the position latitude
                 * @param height the position ellipsoidal height
                 */
                void processPosition(uint64_t microEpoch,double longitude,double latitude,double height){
			positions.push_back(Position(microEpoch,longitude,latitude,height));
                };

                /**
                 * Add the information of a ping in the vector pings
                 * 
                 * @param microEpoch the ping timestamp
                 * @param id the ping id
                 * @param beamAngle the ping beam angle
                 * @param tiltAngle the ping tilt angle
                 * @param twoWayTravelTime the ping two way travel time
                 * @param quality the ping quality
                 * @param intensity the ping intensity
                 */
                void processPing(uint64_t microEpoch,long id, double beamAngle,double tiltAngle,double twoWayTravelTime,uint32_t quality,uint32_t intensity){
			pings.push_back(Ping(microEpoch,id,quality,intensity,currentSurfaceSoundSpeed,twoWayTravelTime,tiltAngle,beamAngle));
                };

                /**
                 * Change the current surface sound speed
                 * 
                 * @param surfaceSoundSpeed the new current surface sound speed
                 */
                void processSwathStart(double surfaceSoundSpeed){
			currentSurfaceSoundSpeed = surfaceSoundSpeed;
                };

                /**
                 * Add a sound velocity profile in the vector svp
                 * 
                 * @param svp the sound velocity profile
                 */
		void processSoundVelocityProfile(SoundVelocityProfile * svp){
			svps.push_back(svp);
		}

                /**Return the georeference (the three ping, the quality and the intensity)*/
		void georeference(){
			//interpolate attitudes and positions around pings
			unsigned int attitudeIndex=0;
			unsigned int positionIndex=0;

			for(auto i=pings.begin();i!=pings.end();i++){
				while(attitudeIndex < attitudes.size() && attitudes[attitudeIndex+1].getTimestamp() < (*i).getTimestamp()){
					attitudeIndex++;
				}

				if(attitudeIndex == attitudes.size() - 1  && attitudes[attitudeIndex+1].getTimestamp() < (*i).getTimestamp()){
					break;
				}

				while(positionIndex < positions.size() && positions[positionIndex+1].getTimestamp() < (*i).getTimestamp()){
					positionIndex++;
				}

				if(positionIndex == positions.size() - 1  && positions[positionIndex+1].getTimestamp() < (*i).getTimestamp()){
					break;
				}

				Attitude & beforeAttitude = attitudes[attitudeIndex];
				Attitude & afterAttitude = attitudes[attitudeIndex+1];

				Position & beforePosition = positions[positionIndex];
				Position & afterPosition  = positions[positionIndex+1];

				Attitude * interpolatedAttitude = Interpolator::interpolateAttitude(beforeAttitude,afterAttitude,(*i).getTimestamp());
				Position * interpolatedPosition = Interpolator::interpolatePosition(beforePosition,afterPosition,(*i).getTimestamp());

				//get SVP for this ping
				SoundVelocityProfile * svp = NULL;

				if(svps.size() == 1){
					svp = svps[0];
				}
				else{
					if(svps.size() > 0){
						//TODO: use a strategy (nearest by time, nearest by location, etc)
						std::cerr << "Multiple SVP mode not yet implemented" << std::endl;
						exit(1);
					}
					else{
						//use default model
						//TODO: allow different models to be used with command line switches
						svp = SoundVelocityProfileFactory::buildSaltWaterModel();
					}
				}

				//TODO: Get leverArm from the data or command line
				Eigen::Vector3d leverArm;
				leverArm << leverArmX,leverArmY,leverArmZ;
                                
                                //georeference
				Eigen::Vector3d georeferencedPing;
				Georeferencing::georeference(georeferencedPing,*interpolatedAttitude,*interpolatedPosition,(*i),*svp,leverArm);

				std::cout << georeferencedPing(0) << " " << georeferencedPing(1) << " " << georeferencedPing(2) << " " << (*i).getQuality()  << " " << (*i).getIntensity() << std::endl;

				delete interpolatedAttitude;
				delete interpolatedPosition;
                                }
			}
                
                /**
                 * set the position x y z of the leverArm
                 * 
                 * @param X the axe x position
                 * @param Y the axe y position
                 * @param Z the axe z position
                 */
                void setLeverArm(double X,double Y,double Z)
                {
                    leverArmX = X;
                    leverArmY = Y;
                    leverArmZ = Z;
                };
                
                /**
                 * Return a text value who contain the x y z value of the leverArm 
                 */
                std::string getleverArmLine()
                {
                    std::stringstream line;
                    line << leverArmX << ":" << leverArmY << ":" << leverArmZ;
                    return line.str();
                }

	private:
                
                /**the current surface sound speed*/
		double 					currentSurfaceSoundSpeed;
                
                /**Vector of pings*/
		std::vector<Ping> 			pings;
                
                /**Vector of positions*/
		std::vector<Position> 			positions;
                
                /**vector of attitudes*/
		std::vector<Attitude> 			attitudes;
                
                /**vector of sound velocity profiles*/
		std::vector<SoundVelocityProfile*>  	svps;
                
                /**the axe x position of the leverArm*/
                double leverArmX = 0.0;
                
                /**the axe y position of the leverArm*/
                double leverArmY = 0.0;
                
                /**the axe z position of the leverArm*/
                double leverArmZ = 0.0;
                
};

/**
  * declare the parser depending on argument receive
  * 
  * @param argc number of argument
  * @param argv value of the arguments
  */
int main (int argc , char ** argv){
	DatagramParser * parser = NULL;
	DatagramGeoreferencer  printer;

#ifdef __GNU__
	setenv("TZ", "UTC", 1);
#endif
#ifdef _WIN32
	putenv("TZ");
#endif
	if(argc < 2){
                std::cout << "Error no file enter";
		printUsage();
	}
        else
        {
	std::string fileName(argv[argc-1]);
        double leverArmX = 0.0;
        double leverArmY = 0.0;
        double leverArmZ = 0.0;

	try{
		std::cerr << "Decoding " << fileName << std::endl;
                std::ifstream inFile;
                inFile.open(fileName);
                if (inFile)
                {    
                    if(ends_with(fileName.c_str(),".all")){
			parser = new KongsbergParser(printer);
                    }   
                    else if(ends_with(fileName.c_str(),".xtf")){
			parser = new XtfParser(printer);
                    }
                    else if(ends_with(fileName.c_str(),".s7k")){
                        parser = new S7kParser(printer);
                    }
                    else{
			throw "Unknown extension";
                    }
                }
                else
                {
                    throw "File not found";
                }
		parser->parse(fileName);
		std::cout << std::setprecision(6);
		std::cout << std::fixed;
                int index;
                while((index=getopt(argc,argv,"x:y:z:"))!=-1)
                {
                    switch(index)
                    {
                        case 'x':
                            if(sscanf(optarg,"%lf", &leverArmX) != 1)
                            {
                                std::cerr << "Invalid lever arm X offset (-x)" << std::endl;
                                std::cout << "Invalid lever arm X offset (-x)";
                                printUsage();
                            }
                        break;
                                        
                        case 'y':
                            if (sscanf(optarg,"%lf", &leverArmY) != 1)
                            {
                                std::cerr << "Invalid lever arm Y offset (-y)" << std::endl;
                                std::cout << "Invalid lever arm Y offset (-y)";
                                printUsage();
                            }
                        break;
                                        
                        case 'z':
                            if (sscanf(optarg,"%lf", &leverArmZ) != 1)
                            {
                                std::cerr << "Invalid lever arm Z offset (-z)" << std::endl;
                                std::cout << "Invalid lever arm Z offset (-z)";
                                printUsage();
                            }
                        break;
                    }
                }
                printer.setLeverArm(leverArmX,leverArmY,leverArmZ);
                printer.georeference();
                std::string leverArmLine;
                leverArmLine = printer.getleverArmLine();
                std::cout << leverArmLine;
        }
	catch(const char * error){
		std::cerr << "Error while parsing " << fileName << ": " << error << std::endl;
                std::cout << "Error while parsing " << fileName << ": " << error;
	}
	if(parser) delete parser;
        }       
}

#endif
