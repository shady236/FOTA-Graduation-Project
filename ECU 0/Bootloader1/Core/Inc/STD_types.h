
#ifndef STD_TYPES_H_
#define STD_TYPES_H_


typedef  unsigned char           u8 ;
typedef  signed char             s8 ;
typedef  unsigned short int      u16;
typedef  signed short int        s16;
typedef  unsigned long int       u32;
typedef  signed long int         s32;
typedef  float                   f32;
typedef  double                  f64;


#define NULL ((void *)0)


typedef  enum
{
	FALSE = 0,
	TRUE
}Boolean_t;


typedef  enum
{
	DISABLE = 0,
	ENABLE
}STD_State_t;


#define  OK        1
#define  NOT_OK    0
#define  ERROR     NOT_OK




#endif /* STD_TYPES_H_ */
