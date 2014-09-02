/*
 * common.h
 *
 *  Created on: Sep 1, 2014
 *      Author: adam
 */

#ifndef COMMON_H_
#define COMMON_H_

#define MAX(a,b) \
	({ typeof (a) _a = (a); \
		typeof (b) _b = (b); \
		_a > _b ? _a : _b; })

#define MIN(a,b) \
	({ typeof (a) _a = (a); \
		typeof (b) _b = (b); \
		_a < _b ? _a : _b; })

#endif /* COMMON_H_ */
