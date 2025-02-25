#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
void initDisplay();      
void dispPublishmqtt();                              
void KettlePage();
void BrewPage();                                                             
void InductionPage();            
void cbpi4kettle_subscribe();
void cbpi4kettle_unsubscribe();
void cbpi4steps_subscribe();
void cbpi4steps_unsubscribe();
void cbpi4notification_subscribe();
void cbpi4notification_unsubscribe();
void cbpi4fermenter_subscribe();
void cbpi4fermenter_unsubscribe();
void cbpi4fermentersteps_subscribe();
void cbpi4fermentersteps_unsubscribe();
void cbpi4kettle_handlemqtt(unsigned char *payload);
void cbpi4sensor_handlemqtt(unsigned char *payload);
void cbpi4steps_handlemqtt(unsigned char *payload);
void cbpi4notification_handlemqtt(unsigned char *payload);
void cbpi4fermenter_handlemqtt(unsigned char *payload);
void cbpi4fermentersteps_handlemqtt(unsigned char *payload);

#endif