#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#define NUM_STICKS 2

int procjoy(int num);
int joyreacquire(int num);
int initjoystick(void);
void uninitjoystick(void);
void clearjoy(void);

typedef struct
{
  int trig;
  int stick;
} tjoystat;
extern tjoystat joystat;

#endif /* JOYSTICK_H_ */
