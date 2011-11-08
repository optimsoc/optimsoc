/*
 * services.h
 *
 *  Created on: Aug 11, 2011
 *      Author: wallento
 */

#ifndef SERVICES_H_
#define SERVICES_H_

unsigned int svc_identify(char* id);

void* svc_req0(unsigned int id,unsigned int req);
void* svc_req1(unsigned int id,unsigned int req,void* arg1);
void* svc_req2(unsigned int id,unsigned int req,void* arg1,void* arg2);
void* svc_req3(unsigned int id,unsigned int req,void* arg1,void* arg2,void* arg3);
void* svc_req4(unsigned int id,unsigned int req,void* arg1,void* arg2,void* arg3,void* arg4);

#endif /* SERVICES_H_ */
