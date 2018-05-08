static long long getDays(int year) {
    long long result;
    if(year<0)return -getDays(-year)-366+365;
    else if(year==0)return 0;
    result=year*365;
    result+=year/4;
    result-=year/100;
    result+=year/400;
    return result;
}

unsigned long date2ut(int year,int month,int date,
        int hour,int minute,int second) {
          
          #define SECONDS_PER_DAY 86400
    const int monthDays[13]={
            0,31,59,90,120,151,181,212,243,273,304,334,365
    };
    unsigned long result;
   
    if(month<1 || month>12)return 0;
    
    result=getDays(year-1)-getDays(1969);
    
    result*=SECONDS_PER_DAY;
    
    result+=monthDays[month-1]*SECONDS_PER_DAY;
    
    if(year%400==0 || (year%4==0 && year%100!=0)) {
        if(month>=3)result+=SECONDS_PER_DAY;
    }
    
    result+=(date-1)*SECONDS_PER_DAY;
    
    result+=hour*60*60;
    
    result+=minute*60;
   
    result+=second;
    
    return result;
}


static int getLeapYearNum(int year) {
    int result;
    if(year<0)return -getLeapYearNum(-year)-1;
    else if(year==0)return 0;
    result=year/4;
    result-=year/100;
    result+=year/400;
    return result;
}
 

static int isLeapYear(int year) {
    if(year%400==0 || (year%4==0 && year%100!=0))return 1;
    return 0;
}
 

void ut2date(volatile ut2date_date_t* result,volatile unsigned long unixTime) {
    int monthDays[13]={
            0,31,59,90,120,151,181,212,243,273,304,334,365
    };
    int i;
    long long days;
    int yearNum;
    int amariDays;
    int daySeconds;
    int hoseiYear;
    if(!result)return;
    if(unixTime>=0) {
        days=unixTime/SECONDS_PER_DAY;
        daySeconds=(int)(unixTime%SECONDS_PER_DAY);
        yearNum=1970+(int)(days/365);
        amariDays=(int)(days%365);
    } else {
        days=(-unixTime)/SECONDS_PER_DAY;
        daySeconds=(int)(SECONDS_PER_DAY-((-unixTime)%SECONDS_PER_DAY));
        if(daySeconds==SECONDS_PER_DAY) {
            daySeconds=0;
        } else days++;
        yearNum=1970-(int)(days/365)-1;
        amariDays=365-(int)(days%365);
    }
    amariDays-=getLeapYearNum(yearNum-1)-getLeapYearNum(1969);
    while(1) {
        if((amariDays>=0 && amariDays<365) ||
            (amariDays==365 && isLeapYear(yearNum)))break;
        if(amariDays<0) {
            hoseiYear=(-amariDays)/365+1;
            amariDays+=hoseiYear*365;
            amariDays+=getLeapYearNum(yearNum-1)-
                getLeapYearNum(yearNum-hoseiYear-1);
            yearNum-=hoseiYear;
        } else if(amariDays>=365) {
            hoseiYear=amariDays/365;
            amariDays=amariDays%365;
            amariDays-=getLeapYearNum(yearNum+hoseiYear-1)-
                getLeapYearNum(yearNum-1);
            yearNum+=hoseiYear;
        } else break;
    }
    if(isLeapYear(yearNum)) {
        for(i=2;i<=12;i++)monthDays[i]++;
    }
    for(i=1;i<=12;i++) {
        if(amariDays>=monthDays[i-1] && amariDays<monthDays[i])break;
    }
    amariDays-=monthDays[i-1];
    result->year=yearNum;
    result->month=i;
    result->date=amariDays+1;
    result->hour=daySeconds/3600;
    result->minute=(daySeconds/60)%60;
    result->second=daySeconds%60;
}





