/*
 * flowtable.c (the flowtable for packet core)
 * AUTHOR: Haowei Shi
 * DATE: October 01, 2014
 *
 */

//Haowei
//#include "flowtable_wrap.c"
#include "ginic_wrap.c"
#include "flowtable.h"
#include "Python.h"
//
void *judgeProcessor(void *pc)
{
    pktcore_t *pcore = (pktcore_t *)pc;
    gpacket_t *in_pkt;
    //flowtable
    SWIG_init();// Haowei: should be init here or in packetcore.c???
    int pktsize;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    while (1)
    {
        verbose(2, "[judgeProcessor]:: Waiting for a packet...");
        //TODO: Redefine data format in Queue from pkt to frame
        readQueue(pcore->decisionQ, (void **)&in_pkt, &pktsize);
        pthread_testcancel();
        verbose(2, "[judgeProcessor]:: Got a packet for further processing...");
        //flow table: Haowei
        //if label is empty, then set it first;
        // if (in_pkt->frame.label[0].process != 2 && in_pkt->frame.label[0].process != 0 && in_pkt->frame.label[0].process != 1)
        // {
        //     labelInit(in_pkt);
        //     switch (ntohs(in_pkt->data.header.prot))
        //     {
        //     case IP_PROTOCOL:
        //         verbose(2, "[packetProcessor]:: Labeling pkt by IP..");
        //         labelNext(in_pkt, NULL_PROTOCOL, IP_PROTOCOL);
        //         writeQueue(pcore->workQ, (void *)in_pkt, sizeof(gpacket_t));//write back to work queue
        //         break;
        //     case ARP_PROTOCOL:
        //         verbose(2, "[packetProcessor]:: Labeling pkt by ARP..");
        //         labelNext(in_pkt, NULL_PROTOCOL, ARP_PROTOCOL);
        //         writeQueue(pcore->workQ, (void *)in_pkt, sizeof(gpacket_t));//write back to work queue
        //         break;
        //     default:
        //         verbose(1, "[packetProcessor]:: Packet discarded: Unknown protocol protocol");
        //         // TODO: should we generate ICMP errors here.. check router RFCs
        //         break;

        //     }
        //     //continue to read next pkt in workq: Haowei
        //     continue;
        // }
        printf("[judgeProcessor]:packet addr:(0x%lx)\n", (unsigned long)in_pkt);

        ftentry_t *entry_res;
        ushort prot;
        printf("[judgeProcessor]:: flowtable size: %d\n", pcore->flowtable->num);
        entry_res = checkFlowTable(pcore->flowtable, in_pkt);
        if (entry_res == NULL)
            //if (!checkFlowTable(pcore->flowtable, in_pkt, action, &prot))
        {
            printf("[judgeProcessor]:: Cannot find action to given packet...Drop!\n");
            return;
        }
        //TODO: call function using action(char *):  PyObject_CallFunction(String)
        printf("[judgeProcessor]:: Entry found protocol: %#06x\n", entry_res->protocol);

        if (entry_res->language == C_FUNCTION)
        {

            verbose(2, "[judgeProcessor]:: C Function: Action: (0x%lx)\n", (unsigned long)entry_res->action);
            int (*processor)(gpacket_t *);
            processor = entry_res->action;
            int nextlabel = (*processor)(in_pkt);
            if (entry_res->protocol == ARP_PROTOCOL || nextlabel == EXIT_SUCCESS){
                verbose(2, "[judgeProcessor] SUCCESS!  : %d\n", EXIT_SUCCESS);
                continue;
            } 
            if (nextlabel == UDP_PROTOCOL) verbose(2, "UDP!!!!!!!!");
            verbose(2, "[Ft]New style round");
            labelNext(in_pkt, entry_res->protocol, nextlabel);
            verbose(2, "Writing back to work Q...");
            writeQueue(pcore->decisionQ, in_pkt, sizeof(gpacket_t));
            verbose(2, "Wrote back to work Q...");
            printSimpleQueue(pcore->decisionQ);

        }
        else if (entry_res->language == PYTHON_FUNCTION)
        {
            verbose(2, "[judgeProcessor]:: Python Function: Action: (0x%lx)\n", (unsigned long)entry_res->action);
            //TODO: Python embedding
            //TODO: ?? Where to declaire!?
            PyObject * Py_pFun, *Py_pPkt, *Py_pResult;
            Py_pFun = entry_res->action;
            Py_pPkt = SWIG_NewPointerObj((void *)in_pkt, SWIGTYPE_p__gpacket_t, 1);
            //Py_pResult = PyObject_CallFunction(Py_pFun, NULL);
            if(Py_pPkt)
            {
                verbose(2, "Got Pyton obj\n");
                Py_pResult = PyObject_CallFunction(Py_pFun, "O", Py_pPkt);
                CheckPythonError();
                printf("pResult: %p",Py_pResult);
            }
        }


        /*
        // get the protocol field within the packet... and switch it accordingly
        switch (ntohs(in_pkt->data.header.prot))
        {
        case IP_PROTOCOL:
            verbose(2, "[packetProcessor]:: Packet sent to IP routine for further processing.. ");

            IPIncomingPacket(in_pkt);
            break;
        case ARP_PROTOCOL:
            verbose(2, "[packetProcessor]:: Packet sent to ARP module for further processing.. ");
            ARPProcess(in_pkt);
            break;
        default:
            verbose(1, "[packetProcessor]:: Packet discarded: Unknown protocol protocol");
            // TODO: should we generate ICMP errors here.. check router RFCs
            break;

        }*/
    }
}
int addEntry(flowtable_t *flowtable, int type, ushort language, void *content)
{
    verbose(2  , "[addEntry]:: \n");
    if (type == CLASSICAL)
    {
        verbose(2  , "[addEntry]:: Adding a classical entry\n");
        //TODO: how to add?: append? insert? : Haowei
        //append: reason: less time when check pkt/add entry, more time delete entry;
        if (flowtable->num < MAX_ENTRY_NUMBER)
        {
            flowtable->entry[flowtable->num].is_empty = 0;
            flowtable->entry[flowtable->num].language = language;
            flowtable->entry[flowtable->num].protocol = UDP_PROTOCOL;// TODO: temporary soluction
            flowtable->entry[flowtable->num].action = content;
            flowtable->num++;
            return EXIT_SUCCESS;
        }
        else
        {
            verbose(2  , "[addEntry]:: flowtable is full...Exit with failure\n");
            return EXIT_FAILURE;
        }


    }
    else if (type == OPENFLOW)
    {
        verbose(2  , "[addEntry]:: Adding a openflow entry\n");
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

int deleteEntry()
{
    verbose(2  , "[deleteEntry]:: \n");
    return EXIT_SUCCESS;
}
flowtable_t *initFlowTable()
{
    verbose(2  , "[initFlowTable]:: \n");
    flowtable_t *flowtable = (flowtable_t *)malloc(sizeof(flowtable_t));
    flowtable->num = 0;
    //int (*function_ptr)(gpacket_t);
    defaultProtocol(flowtable, ARP_PROTOCOL, (void *)ARPProcess);
    defaultProtocol(flowtable, IP_PROTOCOL, (void *)IPIncomingPacket);
    defaultProtocol(flowtable, ICMP_PROTOCOL, (void *)ICMPProcessPacket);
    //default entries IP
    // ftentry_t *entry = (ftentry_t *)malloc(sizeof(ftentry_t));
    // entry->is_empty = 0;
    // entry->language = C_FUNCTION;
    // entry->protocol = IP_PROTOCOL;
    // entry->action = (void*)IPIncomingPacket;
    // flowtable->entry[0] = entry;
    //default entries ARP

    verbose(2  , "[initFlowTable]:: finished size: %d\n", flowtable->num);
    return flowtable;
}
int defaultProtocol(flowtable_t *flowtable, ushort prot, void *function)
{
    verbose(2  , "[defaultProtocol]:: Adding default protocol: %hu\n", prot);
    if (flowtable->num < MAX_ENTRY_NUMBER)
    {
        flowtable->entry[flowtable->num].is_empty = 0;
        flowtable->entry[flowtable->num].language = C_FUNCTION;
        flowtable->entry[flowtable->num].protocol = prot;
        flowtable->entry[flowtable->num].action = function;
        flowtable->num ++;
    }
    else
    {
        verbose(2  , "[defaultProtocol]:: Exceed MAX_ENTRY_NUMBER\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int addProtocol(flowtable_t *flowtable, ushort language, char *protname)
{
    //TODO: 1. import module 2. find getEntry function 3.addEntry with \
    //given entry
    //======
    //SWIG_init();
    //init_CFT();//TODO: build CFT
    //==
    verbose(2, "[addProtocol]Start to add protocol");
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");

    PyObject *pProtMod, *pProtGlobalDict, *pFuncProcess, *pFuncCommand;
    pProtMod = PyImport_ImportModule(protname);//load protocol.py
    if (!pProtMod)
    {
        verbose(2  , "[addProtocol]loading protocol module failed!\n");
        return EXIT_FAILURE;
    }
    //verbose(2  , "Executing Python scritps...\n");

    //verbose(2  , "%s",pUDPMod);
    verbose(2  , "[addProtocol]New protocol Module loaded\n");
    if (pProtMod != NULL)
    {
        verbose(2  , "[addProtocol]module [udp] imported\n");
        pProtGlobalDict = PyModule_GetDict(pProtMod);   // Get main dictionary
        //CheckPythonError();
        if (pProtGlobalDict != NULL)
        {
            verbose(2  , "[addProtocol]main dictionary got\n");
            pFuncProcess = PyDict_GetItemString(pProtGlobalDict, "Protocol_Processor");//TODO: find function of getEntry
            verbose(2  , "[addProtocol]Protocol_Processor got\n");
            pFuncCommand = PyDict_GetItemString(pProtGlobalDict, "Command_Line");
            if(pFuncCommand == NULL) verbose(2  , "[addProtocol]pFuncCommand is NULL!!\n", pFuncCommand);
            verbose(2  , "[addProtocol]Command_Line got\n");
            //return a string for command: 
            PyObject *Py_Config = PyDict_GetItemString(pProtGlobalDict, "Config");
            verbose(2  , "[addProtocol]Config got\n");
            PyObject *Py_String = PyObject_CallFunction(Py_Config, NULL);
            if(Py_String == NULL)
            {
                verbose(2  , "[addProtocol]Py_String is NULL !\n");
            }
            char *command = PyString_AsString(Py_String);
            registerCLI(command, pFuncCommand, PYTHON_FUNCTION, "command", "command", "command");
            
            verbose(2  , "[addProtocol]Command < %p >registered\n", pFuncCommand);
            printf("[addProtocol]Command < %p >registered\n", pFuncCommand);
            //CheckPythonError();
            if (pFuncProcess == NULL) 
            {
                verbose(2, "[addProtocol]pFunc is NULL !!");
                return EXIT_FAILURE;
            }
            addEntry(flowtable, CLASSICAL, language, (void *)pFuncProcess);//add protocol into flow table
            //registerCLI("giniudp", addprotCmd, NULL, NULL, NULL);
            verbose(2, "[addProtocol]!!!!Python Processor added into flowtable!!!");
            /*            if (pFunc)
                        {
                            verbose(2  , "found function [getEntry]\n");
                            //pArg = SWIG_NewPointerObj((void *)&Object, SWIGTYPE_p__gpacket_t, 1);
                            pArg = SWIG_NewPointerObj((void *)in_pkt, SWIGTYPE_p__gpacket_t, 1);//TODO:
                            pPcore = SWIG_NewPointerObj((void *)pcore, SWIGTYPE_p_pktcore_t, 1);//TODO:
                            verbose(2  , "<----[UDPProcess_C]---pPcore@%d , pcore@%d--------->\n", pPcore, pcore);
                            CheckPythonError();
                            if(pArg)
                            {
                                verbose(2  , "Got Pyton obj\n");
                                pResult = PyObject_CallFunction(pFunc, "OO", pArg, pPcore);
                                CheckPythonError();
                                verbose(2  , "pResult: %s",pResult);
                            }
                        }*/
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}
// only check the protocol for now
ftentry_t *checkFlowTable(flowtable_t *flowtable, gpacket_t *pkt)
{
    //ftentry_t *entry_res = (ftentry_t *)malloc(sizeof(ftentry_t));
    //verbose(2  , "[checkFlowTable]:: \n");
    //find the protocol label
    int i, j, fromUpper = 0, prot = NULL_PROTOCOL;
    verbose(2  , "[checkFlowTable]:: Search protocol(EtherType): %#06x\n", ntohs(pkt->data.header.prot));
    for (i = 0; i < 8; i ++)
    {
        if (pkt->frame.label[i].prot != NULL_PROTOCOL && pkt->frame.label[i].process == 0)
        {
            prot = pkt->frame.label[i].prot;
            verbose(2  , "[checkFlowTable]:: Found Next protocol: %hu in pkt: %hu\n", prot, ntohs(pkt->data.header.prot));
            if(pkt->frame.label[i + 1].prot == 1)
            {
                verbose(2  , "[checkFlowTable]:: From Upper Layer of %hu", prot);
                fromUpper = 1;
            }
            break;
        }

    }
    if (prot == NULL_PROTOCOL)
    {
        verbose(2  , "[checkFlowTable]::Didn't find any protocol in FT!");
        return NULL;
    }
    verbose(2  , "[checkFlowTable] size of flowtable: %d\n", flowtable->num);
    for (j = 0; j < flowtable->num; j ++)
    {
        //verbose(2  , "[checkFlowTable]::Checking for entry");
        if (flowtable->entry[j].protocol == prot)
        {
            verbose(2  , "[checkFlowTable]:: Entry found protocol(entry): %#06x\n", flowtable->entry[j].protocol);
            return &(flowtable->entry[j]);
        }
    }
    verbose(2  , "!!!!?????\n");
    return NULL;
}

void printFlowTable(flowtable_t *flowtable)
{
    printf("--  Flow Table Status  --\n");
    printf("Size: %d\n", flowtable->num);
    printf("Details: \n");
    int i;
    for(i = 0;i < flowtable->num; i ++)
    {
        printf("\t[%d]protocol: %d language: %d :: action %p\n", 
            i, 
            flowtable->entry[i].protocol, 
            flowtable->entry[i].language, 
            flowtable->entry[i].action);
    }
    printf("-- End of Flow Table --\n");
}