#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
/*
	Evaluate an octal constant
*/
int octal_toi (const char *parm, int *error)
{
	int ret = 0;
	const char *begparm = parm;
	while (*parm != '\0'){
		char car = *parm++;
		if (isdigit(car) && car < '8'){
			ret = ret * 8 + car - '0';
		}else{
			fprintf (stderr
				,"An octal constant was expected: %s\n"
				,begparm);
			*error = -1;
			break;
		}
	}
	return ret;
}

/*
	Evaluate an hexadecimal constant
*/
int hexa_toi (const char *parm, int *error)
{
	int ret = 0;
	const char *begparm = parm;
	while (*parm != '\0'){
		char car = *parm++;
		if (isdigit(car)){
			ret = ret * 16 + car - '0';
		}else{
			car = tolower (car);
			if (isalpha(car) && car <= 'f'){
				ret = ret * 16 + car - 'a' + 10;
			}else{
				fprintf (stderr
					,"A hexadecimal constant was expected: %s\n"
					,begparm);
				*error = -1;
				break;
			}
		}
	}
	return ret;
}

/*
	Evaluate a decimal constant
*/
int deci_toi (const char *parm, int *error)
{
	int ret = 0;
	const char *begparm = parm;
	while (*parm != '\0'){
		char car = *parm++;
		if (isdigit(car)){
			ret = ret * 10 + car - '0';
		}else{
			fprintf (stderr
				,"A decimal constant was expected: %s\n"
				,begparm);
			*error = -1;
			break;
		}
	}
	return ret;
}

/*
	Evaluate a numeric constant.
	Set error to -1 if anything wrong.
*/
int xtoi (const char *parm, int *error)
{
	int ret = 0;
	if (parm[0] == '0'){
		/* Either octal of hexadecimal */
		if (parm[1] == 'x' || parm[1] == 'X'){
			/* Hexadecimal */
			parm += 2;
			hexa_toi (parm,error);
		}else{
			/* Octal */
			ret = octal_toi (parm,error);
		}
	}else{
		/* Decimal */
		ret = deci_toi (parm,error);
	}
	return ret;
}

