/* 
 * File:   Constants.h
 * Author: andrea
 *
 * Created on 19 settembre 2016, 16.14
 */

#ifndef CONSTANTS_H
#define	CONSTANTS_H

namespace Constant
{
    const int DISTANCE_LINES = 50;    // distanza tra linee della strada/pista
    const int INITIAL_CAR_X = 0;    // posizione iniziale della macchina X
    const int INITIAL_CAR_Z = 480;    // posizione iniziale della macchina Z

    const int INITIAL_TARGET_Z = +470;    // posizione iniziale della macchina Z
    
    // Random limiti per generazione casuale di un GOAL
    const float X_POS_GOAL_MAX = +5.0;
    const float X_POS_GOAL_MIN = -5.0;
    const float Z_POS_GOAL_MAX = -10.0;
    const float Z_POS_GOAL_MIN = -35.0;
    // Random limiti per generazione casuale di un TNT
    // Genero un TNT in posizione piu centrale e più vicino alla macchina in corsa 
    // in modo da mettere in difficoltà il Player
    const float X_POS_TNT_MAX = +0;
    const float X_POS_TNT_MIN = -0.0;
    const float Z_POS_TNT_MAX = -12.0;
    const float Z_POS_TNT_MIN = -17.0;
    // CUBE TARGET DIMENSION
    const float DIM_CUBE = 0.8;
    const float DIM_X_MIN_TARGET = -DIM_CUBE, DIM_X_MAX_TARGET = DIM_CUBE;
    const float DIM_Y_MIN_TARGET = 0.15, DIM_Y_MAX_TARGET = 1.75;
    const float DIM_Z_MIN_TARGET = -DIM_CUBE, DIM_Z_MAX_TARGET = DIM_CUBE;
    // POSIZIONE TORCIA STATUA
    const float POS_TORCIA_X = 0.0;
    const float POS_TORCIA_Y = 70.0;
    const float POS_TORCIA_Z = -440.0;
    // LIMITE CATTURA TARGET
    const float LIMIT_VISIBILITY_TARGET = 5.0;
    // RADAR LENGTH IN PIXEL
    const float RADAR_LENGTH = 250.0;
    // INIZIO Z DELLE LINEE DELLA STRADA
    const float START_Z_STREET_LINES = 500.0;
}

#endif	/* CONSTANTS_H */

