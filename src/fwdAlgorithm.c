/*
 * fwdAlgorithm.c
 *
 *  Created on: May 1, 2015
 *      Author: tsp3859
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fwdAlgorithmHelper.h"
#include "boolean.h"
#include "tierUtils.h"

#define SUCCESS 0
#define ERROR 1

char *fwdInterface = NULL;
char *fwdTierAddr = NULL;
int fwdSet = -1;

extern boolean containsTierAddress(char testStr[20]);
extern int getTierValue(char strCheck[]);
extern boolean setByTierPartial(char inTier[20], boolean setFWDFields);
extern boolean setByTierOnly(char inTier[20], boolean setFWDFields);
extern boolean setByTierManually(char inTier[20], boolean setFWDFields);
extern int getUniqueChildIndex(char strCheck[]);
extern void findParntLongst(char* myTierAdd,char* parentTierAdd);
extern boolean isDestSubstringOfMyLabels(char* destLabel,char* myMatchedLabel);
extern boolean isMyLabelSubstringOfDest(char* destLabel,char* myMatchedLabel);
extern void printNeighbourTable();
extern int  examineNeighbourTable(char* desTierAdd,char* longstMatchingNgbr, int type);

extern FILE *fptr;
extern int enableLogScreen;
extern int enableLogFiles;


int packetForwardAlgorithm(char currentTier[], char desTier[]);
boolean optimusForwardAlgorithm(char currentTier[], char desTier[]);
boolean algorithmOptimusG(char currentTier[], char desTier[],
		int tierValueCurrent);

//Functions created for util later to be moved
void getUID(char* curUID,char* currentTier);
void formNextUIDtoTransferInCase3B(char* nextTierAddress,char* currentTierAddress,boolean cond);
boolean checkIfDestUIDSubStringUID(char* destUID,char* myUID);

/**
 * packetForwardAlgorithm(char[],char[])
 *
 * method to perform packet forwarding
 *
 * @param currentTier (char[]) - current tier address
 * @param desTier     (char[]) - destination tier address
 *
 * @return returnValue   (int) - algorithm return value (-1/0/1)
 */
int packetForwardAlgorithm(char myTierAdd[], char desTierAdd[]) 
{

	if(enableLogScreen)
		printf("\n\n********************Entering packetForwardAlgorithm********************\n");
	if(enableLogFiles)
		fprintf(fptr,"\n\n********************Entering packetForwardAlgorithm********************\n");
    //printNeighbourTable(); // added on August 25
	int returnValue = ERROR;
	boolean checkOFA = false;



	// Case:1 If( Destination Label == My Label )
	int chkDestLbl = ERROR;

	//check whether the packet reached the destination by checking all the labels of the node
	chkDestLbl = CheckAllDestinationLabels(desTierAdd);

	if (chkDestLbl == SUCCESS) {

		// Case:1 Current Tier  = Destination Tier
		if (enableLogScreen){
			printf("\nCase:1 [TRUE] My Label [%s] = Destination Label [%s] \n", myTierAdd, desTierAdd);
		}
		boolean checkIfFWDSet =setByTierManually(desTierAdd,true);// Change the function name to give a relevant one so we know what it does.

		if (checkIfFWDSet == true)
		{
			if(enableLogScreen)
				printf("\nPacket sent to the ipnode successfully"); 
			if(enableLogFiles)
				fprintf(fptr,"\nPacket sent to the ipnode successfully"); 
			checkOFA = true;  
			fwdSet = SUCCESS; 
			returnValue = SUCCESS;
		}
		else
		{
			if(enableLogScreen)
				printf("\nCase:1:ERROR: Failed to set FWD Tier Address\n");
			if(enableLogFiles)
				fprintf(fptr,"\nCase:1:ERROR: Failed to set FWD Tier Address\n");
			fwdSet = ERROR;
			returnValue = ERROR;
		}
	}
	else 
	{
		if(enableLogScreen)
			printf("\nCase:1 [NOT TRUE] Destination label [%s] not in my label list \n",desTierAdd);
		if(enableLogFiles)
			fprintf(fptr,"\nCase:1 [NOT TRUE]  Destination label [%s] not in my label list \n",desTierAdd);
		// Check for Case 2 : if Destinaton label is in my neighbour table
		if (containsTierAddress(desTierAdd) == true)
		{
			// Case2 : Destinaton label is in my neighbour table
			if(enableLogScreen)
				printf("\nCase:2 [TRUE] Destinaton label is in my neighbour table \n");
			if(enableLogFiles)
				fprintf(fptr,"\nCase:2 [TRUE] Destinaton label is in my neighbour table \n");

			//Forward Packet to the port curresponding  to the Destination label
			returnValue = setNextTierToSendPacket(desTierAdd); // Change the name of the function.

		}
		else
		{
			if(enableLogScreen)
 				printf("\nCase:2 [NOT TRUE] Destinaton label is in my neighbour table \n");
 			if(enableLogFiles)
				fprintf(fptr,"\nCase:2 [NOT TRUE] Destinaton label is in my neighbour table \n");
			int myTierValue =  getTierVal(myTierAdd);
			int destTierValue = getTierVal(desTierAdd);
			
			//Case 3 : If ( (My Tier Value ==  Destination Tier Value)  && (Tier Value !=1) ) 

			if( (myTierValue == destTierValue) && (myTierValue != 1))
			{
				if(enableLogScreen)
					printf("\nCase:3 [TRUE] My Tier Value ==  Destination Tier Value && Tier Value !=1 \n");

                int doesNTentryMatchDest = 0;

				char parentTierAdd[20];
				memset(parentTierAdd,'\0',20);
				//printNeighbourTable(); // Commented on August 25, 2017
				// if(enableLogScreen)
 			// 		printf("\nFinding my parent by comparing my primary label for substring match\n");
 			// 	if(enableLogFiles)
				// 	fprintf(fptr,"\nFinding my parent by comparing my primary label for substring match\n");

				// findParntLongst(myTierAdd,parentTierAdd); // Commented by supriya on August 30, 2017.

				char longstMatchingNgbr[20];
				memset(longstMatchingNgbr,'\0',20);

				if(enableLogScreen)
 					printf("\nFinding a common parent by checking if there is a longest substring match in between the destination label [%s] and my neighbor table labels\n",desTierAdd);
 				if(enableLogFiles)
					fprintf(fptr,"\nFinding a common parent by checking if there is a longest substring match in between the destination label [%s] and my neighbor table labels\n",desTierAdd);
				//success if there is a longest substring match between the neighbor table entries and destination tier address
				doesNTentryMatchDest = examineNeighbourTable(desTierAdd,longstMatchingNgbr,1); // make change in the funciton, check only for parent nodes.
                if(doesNTentryMatchDest == SUCCESS){
					returnValue = setNextTierToSendPacket(longstMatchingNgbr);
                }
                else {
                    char *parentTierAddress;
                    //	memset(parentTierAddress,'\0',20);

                    char tempMyTierAddress[20];
                    memcpy(tempMyTierAddress, myTierAdd, strlen(myTierAdd) + 1);
                    if (enableLogScreen)
                        printf("\nNo common parent found.\nGenerating parent label from my label = %s to forward the packet to. \n", tempMyTierAddress);
                    if (enableLogFiles)
                        fprintf(fptr, "\nNo common parent found.\nGenerating parent label from my label = %s to forward the packet to. \n", tempMyTierAddress);
                    //trying to get the parent
                    parentTierAddress = getParent(tempMyTierAddress, '.');
                    if (enableLogScreen)
                        printf("\nGenerated parent label is [%s]\n", parentTierAddress);
                 	
                    returnValue = setNextTierToSendPacket(parentTierAddress);
                }
			}
			else
			{
			//Case4 and 5: if my tv is not equal to dest. tv
				if(enableLogScreen)
					printf("Case:3 [NOT TRUE] My Tier Value =  Destination Tier Value && Tier Value !=1 \n");
				if(enableLogFiles)
					fprintf(fptr,"Case:3 [NOT TRUE] My Tier Value =  Destination Tier Value && Tier Value !=1 \n");

				printf("My Tier address: %s \n",myTierAdd);
				printf("Destination Tier address: %s \n",desTierAdd);
				printf("My Tier value: %d \n",myTierValue);
				printf("Destination Tier value: %d \n",destTierValue);

				char destUID[20];
				char myUID[20];

				getUID(myUID,myTierAdd);
				getUID(destUID,desTierAdd);
				
				// char parentTierAdd[20];
				// memset(parentTierAdd,'\0',20);
				// //printNeighbourTable(); // Commented on August 25, 2017
				// findParntLongst(myTierAdd,parentTierAdd);


				if(myTierValue != destTierValue)
				{
					//case 4
					if(myTierValue > destTierValue)
					{
						if(enableLogScreen)
							printf("\nCase:4 [TRUE] My Tier Value !=  Destination Tier Value && My TV > Dest. TV");
						if(enableLogFiles)
							fprintf(fptr,"\nCase:4 [TRUE] My Tier Value !=  Destination Tier Value && My TV > Dest. TV");
						
						if(enableLogScreen)
							printf("\nChecking if the destination UID is a substring of any of my UIDs");
						if(enableLogFiles)
							fprintf(fptr,"\nChecking if the destination UID is a substring of any of my UIDs");


						char myMatchedLabel[20];
						memset(myMatchedLabel,'\0',20);
						boolean check = isDestSubstringOfMyLabels(desTierAdd,myMatchedLabel);


						if(check == true)
						{	
							if(enableLogScreen){
								printf("\nisDestSubstringOfMyLabels = TRUE\nDestination label [%s] is a substring of my label [%s]. Hence it is either my parent or grandparent.",desTierAdd,myMatchedLabel);
								// printf("\n Sending packet to parent %s\n", parentTierAdd);
							}
							if(enableLogFiles){
								fprintf(fptr,"\nisDestSubstringOfMyLabels = TRUE\nDestination label [%s] is a substring of my label [%s]. Hence it is either my parent or grandparent.",desTierAdd,myMatchedLabel);
								// fprintf(fptr,"\n Sending packet to parent %s\n", parentTierAdd);
							}

							if(enableLogScreen){
								printf("\nGet the parent of my label [%s] to forward the packet to",myMatchedLabel);
								// printf("\n Sending packet to parent %s\n", parentTierAdd);
							}
							if(enableLogFiles){
								fprintf(fptr,"\nGet the parent of my label [%s] to forward the packet to",myMatchedLabel);
								// fprintf(fptr,"\n Sending packet to parent %s\n", parentTierAdd);
							}

							char *parentTierAddress;
							char tempMyTierAddress[20];
		                    memcpy(tempMyTierAddress, myMatchedLabel, strlen(myMatchedLabel) + 1);
		                    parentTierAddress = getParent(tempMyTierAddress, '.');
		                    if (enableLogScreen)
		                        printf("\nForwarding the packet to parent: [%s]\n", parentTierAddress);

		                    if(enableLogFiles){
								fprintf("\nForwarding the packet to parent: [%s]\n", parentTierAddress);
								// fprintf(fptr,"\n Sending packet to parent %s\n", parentTierAdd);
							}
		                    returnValue = setNextTierToSendPacket(parentTierAddress);
						}
						else
						{
							if(enableLogScreen)
								printf("\nisDestSubstringOfMyLabels = FALSE\nDestination label is NOT a substring of my label.");
							if(enableLogFiles)
								fprintf(fptr,"\nisDestSubstringOfMyLabels = FALSE\nDestination label is NOT a substring of my label.");
							char longstMatchingNgbr[20];
							memset(longstMatchingNgbr,'\0',20);

							//printNeighbourTable(); // Commented on August 25, 2017
							int isDestUIDSubNeigbUID = examineNeighbourTable(desTierAdd,longstMatchingNgbr,2);// check if we are checking the destination is a substring(parent) of any of my neighbor
							
							//if not success , set the next node to my parent
							if(isDestUIDSubNeigbUID != SUCCESS){
								if(enableLogScreen)
									printf("\nDestination label not a substring of any neighbour.");
								if(enableLogFiles)
									fprintf(fptr,"\nDestination label not a substring of any neighbour.");
								char parentTierAdd[20];
								memset(parentTierAdd,'\0',20);
								findParntLongst(myTierAdd,parentTierAdd);
								strcpy(longstMatchingNgbr,parentTierAdd);
								if(enableLogScreen)
									printf("\nSending the packet to parent: %s\n",parentTierAdd);
								if(enableLogFiles)
									fprintf(fptr,"\nSending the packet to parent: %s\n",parentTierAdd);
							}
							else{
								if(enableLogScreen)
									printf("\nSending the packet to the longest matching neighbour %s",longstMatchingNgbr);
								if(enableLogFiles)
									fprintf(fptr,"\nSending the packet to the longest matching neighbour %s",longstMatchingNgbr);
							}
							returnValue = setNextTierToSendPacket(longstMatchingNgbr);
						}
					}
					//case 5
					else 
					{
						if(enableLogScreen)
							printf("\nCase:4 [NOT TRUE] My Tier Value !=  Destination Tier Value && My TV > Dest. TV\nCase:5 [TRUE] My Tier Value !=  Destination Tier Value && My TV < Dest. TV");	
						if(enableLogFiles)
							fprintf(fptr,"\nCase:4 [NOT TRUE] My Tier Value !=  Destination Tier Value && My TV > Dest. TV\nCase:5 [TRUE] My Tier Value !=  Destination Tier Value && My TV < Dest. TV");	
						//boolean check = checkIfDestUIDSubStringUID(destUID,myUID);
						
						if(enableLogScreen)
							printf("\nChecking if any of my lables is a substring of the destination label");
						if(enableLogFiles)
							fprintf(fptr,"\nChecking if any of my lables is a substring of the destination label");

						char myMatchedLabel[20];
						memset(myMatchedLabel,'\0',20);
						boolean check = isMyLabelSubstringOfDest(desTierAdd,myMatchedLabel);

						if(check == true)
						{	
							if(enableLogScreen)
								printf("\nisMyLabelSubstringOfDest = TRUE\nMy label [%s] is a substring of Destination label [%s]", myMatchedLabel, desTierAdd);
							if(enableLogFiles)
								fprintf(fptr,"\nisMyLabelSubstringOfDest = TRUE\nMy label [%s] is a substring of Destination label [%s]", myMatchedLabel, desTierAdd);
							//Forward packet to my child with the longest substring match
							if(enableLogScreen)
								printf("\nThis means destination node is my child or grand child.");
							if(enableLogFiles)
								fprintf(fptr,"\nThis means destination node is my child or grand child.");

							char childTierAdd[20];
							memset(childTierAdd,'\0',20);

							//printNeighbourTable(); // Commented on August 25, 2017
							findChildLongst(desTierAdd,childTierAdd,myMatchedLabel);
							
							if(enableLogScreen){
								//printf("\n checkIfDestUIDSubStringUID = TRUE \n");
								printf("\nSending the packet to longest matching child: %s\n",childTierAdd);
							}
							if(enableLogFiles){
								//fprintf(fptr,"\n checkIfDestUIDSubStringUID = TRUE \n");
								fprintf(fptr,"\nSending the packet to longest matching child: %s\n",childTierAdd);
							}
							returnValue = setNextTierToSendPacket(childTierAdd);
						}
						else
						{
							if(enableLogScreen)
   								printf("\nisMyLabelSubstringOfDest = FALSE\nMy label is NOT a substring of Destination label [%s]\nExamining the Neighbor table to check if any of the neighbor is substring of the destination label.", desTierAdd);
							if(enableLogFiles)
   								fprintf(fptr,"\nisMyLabelSubstringOfDest = FALSE\nMy label is NOT a substring of Destination label [%s]\nExamining the Neighbor table to check if any of the neighbor is substring of the destination label.", desTierAdd);
							char longstMatchingNgbr[20];
							memset(longstMatchingNgbr,'\0',20);
							//printNeighbourTable(); // Commented on August 25, 2017
							int isDestUIDSubNeigbUID = examineNeighbourTable(desTierAdd,longstMatchingNgbr,1); // check the function again, check if any of the neighbors is substring of destination label.
							
							//if not success , set the next node to my parent
							if(isDestUIDSubNeigbUID != SUCCESS){
								char parentTierAdd[20];
								memset(parentTierAdd,'\0',20);
								findParntLongst(myTierAdd,parentTierAdd);
								strcpy(longstMatchingNgbr,parentTierAdd);
								if(enableLogScreen){
									//printf("\n checkIfDestUIDSubStringUID = FALSE \n");
		                        	printf("\nNO neighbor is a substring of the destination label\nForwarding the packet to my parent: %s \n",longstMatchingNgbr);
		                    	}
		                        if(enableLogFiles){
	   								//fprintf(fptr,"\n checkIfDestUIDSubStringUID = FALSE \n");
	   								fprintf(fptr,"\nNO neighbor is a substring of the destination label\nForwarding the packet to my parent:  %s \n",longstMatchingNgbr);
	   							}
							}
							else{
								if(enableLogScreen){
									//printf("\n checkIfDestUIDSubStringUID = FALSE \n");
		                        	printf("\nSending the packet to longest matching neighbour: %s \n",longstMatchingNgbr);
		                    	}
		                        if(enableLogFiles){
	   								//fprintf(fptr,"\n checkIfDestUIDSubStringUID = FALSE \n");
	   								fprintf(fptr,"\nSending the packet to longest matching neighbour %s \n",longstMatchingNgbr);
	   							}
   							}
	                        returnValue = setNextTierToSendPacket(longstMatchingNgbr);
							
						}

					}	
				}
			}
		}		
	}
	if(enableLogScreen)
		printf("\n\n%s:Exit , returnValue = %d \n",__FUNCTION__,returnValue);
	if(enableLogFiles)
   		fprintf(fptr,"\n\n%s:Exit , returnValue = %d \n",__FUNCTION__,returnValue);
	return returnValue;
}


/**
 * setNextTierToSendPacket(char[])
 *
 * method to set the address of next node to send the packet
 *
 * @param nodeAddress (char[]) - destination  node address
 * @return returnValue   (int) - algorithm return SUCCESS if it is able to
 									else ERROR
 */

int setNextTierToSendPacket(char* nodeAddress)
{
	int returnValue = ERROR;
	//sending the packet from the current node to the next node
	boolean checkFWDSet = setByTierOnly(nodeAddress, true);
	
	if (checkFWDSet == true)
	{
		if(enableLogScreen)
			printf("\ncheckFWDSet == true , setting the fwdSet \n");
		if(enableLogFiles)
   			fprintf(fptr,"\ncheckFWDSet == true , setting the fwdSet \n");
		fwdSet = SUCCESS; //to-do need of this variable ?
		returnValue = SUCCESS;
	} 
	else 
	{
		if(enableLogScreen)
			printf("\nERROR: Failed to set to the parent Tier Address\n");
		if(enableLogFiles)
   			fprintf(fptr,"\nERROR: Failed to set to the parent Tier Address\n");
		returnValue = ERROR;
		fwdSet = ERROR; //to-do need of this variable ?
	}
	return returnValue;

}


/**
 * checkIfDestUIDSubStringUID(char[],char[])
 *
 * method to check if the destination UID is a substring of my UID
 *
 * @param destUID (char[]) - destination  UID
 * @param myUID   (char[]) - current UID
 *
 * @return returnValue   (boolean) - algorithm return true if it is
 									else false
 */

boolean checkIfDestUIDSubStringUID(char* destUID,char* myUID)
{
	if(enableLogScreen)
		printf("\n myUID = %s destUID=%s",myUID,destUID);
	if(enableLogFiles)
   		fprintf(fptr,"\n myUID = %s destUID=%s",myUID,destUID);

	//3.1 //1

	int pos1 = 0;
	int pos2 = 0;

	while(destUID[pos1] != '\0' && myUID[pos2] != '\0'){

		int destVal = 0;

		while(destUID[pos1] != '\0' && destUID[pos1] != '.'){

		 	destVal = destVal * 10 + destUID[pos1] - '0';
		 	pos1++;
		}
		pos1++;

		int myVal = 0;

		while(myUID[pos2] != '\0' && myUID[pos2] != '.'){

		 	myVal = myVal * 10 + myUID[pos2] - '0';
		 	pos2++;
		}
		pos2++;
		
		if(enableLogScreen)
			printf("\n destVal =%d myVal=%d",destVal,myVal);
		if(enableLogFiles)
   			fprintf(fptr,"\n destVal =%d myVal=%d",destVal,myVal);

		if(destVal != myVal){
			if(enableLogScreen)
				printf("\n False");
			if(enableLogFiles)
   				fprintf(fptr,"\n False");
			return false;
		}

	}
	if(enableLogScreen)
		printf("\n True");
	if(enableLogFiles)
   		fprintf(fptr,"\n True");
	return true;

}


/**
 * formNextUIDtoTransferInCase3B
 *
 * Method that forms the next UID to transfer in case of 3B
 *
 * @return boolean
 */

void formNextUIDtoTransferInCase3B(char* nextTierAddress ,char* currentTierAddress,boolean cond  ){

	int i = strlen(currentTierAddress)-1; 
	int k = 0;
	int savePos = 0;
	if(enableLogScreen)
		printf("\n formNextUIDtoTransferInCase3B : currentTierAddress = %s condition = %s \n",currentTierAddress,
								(cond == true)?"true":"false");
	if(enableLogFiles)
   		fprintf(fptr,"\n formNextUIDtoTransferInCase3B : currentTierAddress = %s condition = %s \n",currentTierAddress,
								(cond == true)?"true":"false");

	//currentTierAddress = 1.1

	strcpy(nextTierAddress,currentTierAddress);

	//nextTierAddress = 1.1

	i = strlen(nextTierAddress)-1;

	//i = 2
	
	while(nextTierAddress[i-1] != '.'){
		i--;
	}

	//i = 2

	savePos = i;

	//SavePos = 2

	k = 0;
	char temp[20];
	memset(temp,'\0',20);

	//temp = "" 
	//i = 2 

	while(i < strlen(nextTierAddress))
	{
		temp[k] = nextTierAddress[i];
		k++;
		i++;
	}

	//temp = "1"

	int tempPart = atoi(temp);

	//tempPart = 1

	//case 3B: +1 case , cond = true
	if(cond) 
	{
		//tempPart = 2
		tempPart++;
	}
	else
	{
	//case 3B: -1 case , cond = true
		tempPart--;
	}

	//temp = itoa(tempPart);

	sprintf(temp, "%d", tempPart);

	//temp = "2"

	k = 0;

	//k= 0 strlen(temp) = 2
	while(k < strlen(temp))
	{
		nextTierAddress[savePos] = temp[k];
		k++;
		savePos++;
	}
	//nextTierAddress = 1.2
	if(enableLogScreen)
		printf("\n%s : nextTierAddress = %s Length=%d\n",__FUNCTION__,nextTierAddress,(int)strlen(nextTierAddress));
	if(enableLogFiles)
   		fprintf(fptr,"\n%s : nextTierAddress = %s Length=%d\n",__FUNCTION__,nextTierAddress,(int)strlen(nextTierAddress));

}


/**
 * compareUIDs(A,B)
 *
 * Method that compares two given UIDs (A and B) and returns true if A < B and False if A > B
 *
 * @return boolean
 */

boolean compareUIDs(char* curUID,char* destUID) {

	//compare the UID's of both current NOde and the destination node
	int ic =0;
	int id = 0;
	char curPart[20];
	char destPart[20];
	int k;
	if(enableLogScreen)
		printf("\n%s : curUID =%s , destUID =%s",__FUNCTION__,curUID,destUID);
	if(enableLogFiles)
   		fprintf(fptr,"\n%s : curUID =%s , destUID =%s",__FUNCTION__,curUID,destUID);

	while( curUID[ic] != '\0' && destUID[id] != '\0' ){

		k  =0;
		while(curUID[ic]  != '\0' && curUID[ic] != '.'){
			curPart[k++] = curUID[ic];
			ic++;
		}
		curPart[k++] = '\0';

		k  =0;
		while(destUID[id] != '\0' && destUID[id] != '.'){
			destPart[k++] = destUID[id];
			id++;
		}
		destPart[k++] = '\0';

		int curPartVal = atoi(curPart);
		int destPartVal = atoi(destPart);
		if(enableLogScreen)
			printf("\n %s: comparing UIDS curPartVal=%d destPartVal=%d ",__FUNCTION__,curPartVal,destPartVal);
		if(enableLogFiles)
   			fprintf(fptr,"\n %s: comparing UIDS curPartVal=%d destPartVal=%d ",__FUNCTION__,curPartVal,destPartVal);

		if(curPartVal < destPartVal){
			if(enableLogScreen)
				printf("\n%s : curPartVal < destPartVal",__FUNCTION__);
			if(enableLogFiles)
   				fprintf(fptr,"\n%s : curPartVal < destPartVal",__FUNCTION__);
			return true;
		}
		else if(curPartVal > destPartVal){
			if(enableLogScreen)
				printf("\n%s : curPartVal < destPartVal",__FUNCTION__);
			if(enableLogFiles)
   				fprintf(fptr,"\n%s : curPartVal < destPartVal",__FUNCTION__);
			return false;
		}
		else{
			//equal case
			//continue
			ic++;
			id++;
			memset(curPart,'\0',20);

			memset(destPart,'\0',20);
		}
	}

	if(destUID[id] != '\0' ){
		if(enableLogScreen)
			printf("%s : destUID is still left",__FUNCTION__);
		if(enableLogFiles)
   			fprintf(fptr,"%s : destUID is still left",__FUNCTION__);
		return false;	
	}
	if(enableLogScreen)
		printf("%s: Some error!! ",__FUNCTION__);
	if(enableLogFiles)
   		fprintf(fptr,"%s: Some error!! ",__FUNCTION__);
	return true; //Should never come to this case as destID is always > curID length

}

/**
 * getUID()
 *
 * method to get the UID from the current Tier address and store it in curUID.
 *
 * @return none
 */

void getUID(char* curUID,char* currentTier){

	int i = 0;
	////Truncate and store the truncated part as the Tier value the UID's of both the current and the destination

	while(currentTier[i] != '.'){
		i++;

	}
	i = i+1;

	int k = 0;

	while(currentTier[i] != '\0'){
			curUID[k] = currentTier[i];
			i++;
			k++;

	}
	curUID[k] = '\0';
}


/**
 * getTierVal()
 *
 * method to get the Tier value from the passed Tier address and return the same.
 *
 * @return int [ the tier it belongs to]
 */

int getTierVal(char* tierAdd)
{
	int i = 0;
	char tierValInString[20];
	int tier = -1;
	memset(tierValInString,'\0',20);
	
	while(tierAdd[i] != '.')
	{
		tierValInString[i] = tierAdd[i];
		i++;
	}

	tier = atoi(tierValInString);
	return tier;
}

/**
 * isFWDFieldsSet()
 *
 * method to check whether forwarding fields are set or not
 *
 * @return fwdSet  (int)- forwarding field status
 */
int isFWDFieldsSet() {

	return fwdSet;
}


