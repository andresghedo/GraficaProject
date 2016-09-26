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
    /* distanza tra linee stradali */
    const int DISTANCE_LINES = 50;
    /* posizione iniziale Car X */
    const int INITIAL_CAR_X = 0;
    /* posizione iniziale Car Z */
    const int INITIAL_CAR_Z = +480;
    /* posizione inizale Goal Z */
    const int INITIAL_TARGET_Z = +470;
    /* limiti Random per generazione casuale di un Goal */
    const float X_POS_GOAL_MAX = +5.0;
    const float X_POS_GOAL_MIN = -5.0;
    const float Z_POS_GOAL_MAX = -7.0;
    const float Z_POS_GOAL_MIN = -20.0;
    /* limiti Random per generazione casuale di un Tnt */
    const float X_POS_TNT_MAX = +0;
    const float X_POS_TNT_MIN = -0.0;
    const float Z_POS_TNT_MAX = -5.0;
    const float Z_POS_TNT_MIN = -8.0;
    /* dimesioni di un Cubo Target o Tnt */
    const float DIM_CUBE = 0.8;
    const float DIM_X_MIN_TARGET = -DIM_CUBE, DIM_X_MAX_TARGET = DIM_CUBE;
    const float DIM_Y_MIN_TARGET = 0.15, DIM_Y_MAX_TARGET = 1.75;
    const float DIM_Z_MIN_TARGET = -DIM_CUBE, DIM_Z_MAX_TARGET = DIM_CUBE;
    /* dimesioni di un Rettangolino per retromarcia */
    const float DIM_X_MIN_REVERSE_LIGHT = -0.05, DIM_X_MAX_REVERSE_LIGHT = +0.05;
    const float DIM_Y_MIN_REVERSE_LIGHT = -0.02, DIM_Y_MAX_REVERSE_LIGHT = 0.02;
    const float DIM_Z_MIN_REVERSE_LIGHT = -0.005, DIM_Z_MAX_REVERSE_LIGHT = 0.005;
    /* posizione torcia XYZ */
    const float POS_TORCIA_X = 0.0;
    const float POS_TORCIA_Y = 70.0;
    const float POS_TORCIA_Z = -440.0;
    /* limite visibilità di un target */
    const float LIMIT_VISIBILITY_TARGET = 5.0;
    /* lunghezza del radar */
    const float RADAR_LENGTH = 250.0;
    /* inizio delle linee stradali Z */
    const float START_Z_STREET_LINES = 500.0;
    /* probabilità di un Goal invece che di un Tnt */
    const float PROBABILITY_OF_GOAL = 0.70; // 0.75
    /* secondi a disposizione dell'utente */
    const float GAME_LIMIT_SECONDS = 100.0;
    /* id univoci delle texture */
    const int TEXTURE_ID_LOGO_GOMME     = 0;
    const int TEXTURE_ID_CARROZZERIA    = 1;
    const int TEXTURE_ID_SKY            = 2;
    const int TEXTURE_ID_TNT            = 3;
    const int TEXTURE_ID_ASFALTO        = 4;
    const int TEXTURE_ID_ERBA           = 5;
    const int TEXTURE_ID_SELFIE         = 6;
    const int TEXTURE_ID_RACING_FLAG    = 7;
    const int TEXTURE_ID_LIGHT_OFF      = 8;
    const int TEXTURE_ID_LIGHT_ON       = 9;
    const int TEXTURE_ID_INFO_USER      = -1;
    /* limiti di posizione macchina nel mondo XZ */
    const float CAR_LIMIT_X             = 20.0;
    const float CAR_LIMIT_Z_SUP         = 500.0;
    const float CAR_LIMIT_Z_INF         = -438.0;
    /* parametro di scala per la statua */
    const float STATUE_SCALE_PARAMETR   = 60.0;
    /* limite oltre il quale non devo più generare target */
    const float LIMIT_GENERATE_TARGET = -425.0;
}

#endif	/* CONSTANTS_H */

