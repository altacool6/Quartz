// UsageSample.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//
#include "QuartzDB.h"
#include <process.h>  
#include "stdio.h"
#include "Windows.h"

void DB_Thread(DB_HANDLE hQDB) 
{
    while(1) {
        QDB_Run(hQDB);
        Sleep(100);
    }
}

static void _CreateDBScheme(DB_HANDLE hQDB)
{

}

static void _BulkInsert(DB_HANDLE hQDB)
{
    QUARTZ_REQ Req;

    INIT_QUARTZ_REQ(Req);

    QDB_InsertRequest(hQDB, &Req);
}

static void _SearchRecord(DB_HANDLE hQDB)
{

}


int main(void)
{
    DB_HANDLE hQDB;

    hQDB = QDB_Open("test.bin");

    _beginthread( DB_Thread, 0, hQDB); //start DB Thread

    _CreateDBScheme(hQDB);

    _BulkInsert(hQDB);

    _SearchRecord(hQDB);

    while(1)
        Sleep(100);

    return 0;
}

