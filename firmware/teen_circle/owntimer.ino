static long long getDays(int year) {
    long long result;
    if(year<0)return -getDays(-year)-366+365;
    else if(year==0)return 0;
    result=year*365;/*1蟷ｴ縺ｯ蝓ｺ譛ｬ365譌･*/
    result+=year/4;/*4縺ｧ蜑ｲ繧雁��繧後◆繧蛾柾蟷ｴ*/
    result-=year/100;/*100縺ｧ蜑ｲ繧雁��繧後◆繧蛾柾蟷ｴ縺ｧ縺ｯ縺ｪ縺�*/
    result+=year/400;/*400縺ｧ蜑ｲ繧雁��繧後◆繧蛾柾蟷ｴ*/
    return result;
}

unsigned long date2ut(int year,int month,int date,
        int hour,int minute,int second) {
          /*1譌･縺ｮ遘呈焚(=24*60*60)*/
          #define SECONDS_PER_DAY 86400
    const int monthDays[13]={
            0,31,59,90,120,151,181,212,243,273,304,334,365
    };/*縺昴�ｮ譛医∪縺ｧ縺ｮ譌･謨ｰ縺ｮ蜥鯉ｼ育ｴｯ遨榊柱�ｼ�*/
    unsigned long result;
    /*繧｢繧ｯ繧ｻ繧ｹ驕募渚縺ｫ縺ｪ繧倶ｸ肴ｭ｣蜈･蜉帙↑繧�0繧定ｿ斐☆*/
    if(month<1 || month>12)return 0;
    /* year蟷ｴ1譛�1譌･ 00:00:00縺ｾ縺ｧ縺ｮ譌･謨ｰ繧呈ｱゅａ繧�*/
    result=getDays(year-1)-getDays(1969);
    /*遘偵↓螟画鋤*/
    result*=SECONDS_PER_DAY;
    /*譛医�ｮ譌･謨ｰ繧堤ｧ偵↓螟画鋤縺励※雜ｳ縺�*/
    result+=monthDays[month-1]*SECONDS_PER_DAY;
    /*髢丞ｹｴ縺九▽3譛井ｻ･髯阪↑繧�1譌･雜ｳ縺�*/
    if(year%400==0 || (year%4==0 && year%100!=0)) {
        if(month>=3)result+=SECONDS_PER_DAY;
    }
    /*縺昴�ｮ譛医�ｮ譌･謨ｰ繧堤ｧ偵↓螟画鋤縺励※雜ｳ縺�*/
    result+=(date-1)*SECONDS_PER_DAY;
    /*譎る俣繧定ｶｳ縺�*/
    result+=hour*60*60;
    /*蛻�繧定ｶｳ縺�*/
    result+=minute*60;
    /*遘偵ｒ雜ｳ縺�*/
    result+=second;
    /*邨先棡繧定ｿ斐☆*/
    return result;
}

/*1蟷ｴ縺九ｉyear蟷ｴ縺ｾ縺ｧ縺ｮ髢丞ｹｴ縺ｮ蛟区焚繧呈ｱゅａ繧�*/
static int getLeapYearNum(int year) {
    int result;
    if(year<0)return -getLeapYearNum(-year)-1;
    else if(year==0)return 0;
    result=year/4;/*4縺ｧ蜑ｲ繧雁��繧後◆繧蛾柾蟷ｴ*/
    result-=year/100;/*100縺ｧ蜑ｲ繧雁��繧後◆繧蛾柾蟷ｴ縺ｧ縺ｯ縺ｪ縺�*/
    result+=year/400;/*400縺ｧ蜑ｲ繧雁��繧後◆繧蛾柾蟷ｴ*/
    return result;
}
 
/*縺昴�ｮ蟷ｴ縺碁柾蟷ｴ縺ｧ縺ゅｋ縺九ｒ豎ゅａ繧�*/
static int isLeapYear(int year) {
    if(year%400==0 || (year%4==0 && year%100!=0))return 1;
    return 0;
}
 
/*1970蟷ｴ1譛�1譌･縲�00:00:00縺九ｉ縺ｮ邨碁℃遘呈焚繧呈律莉倥↓縺吶ｋ*/
void ut2date(volatile ut2date_date_t* result,volatile unsigned long unixTime) {
    int monthDays[13]={
            0,31,59,90,120,151,181,212,243,273,304,334,365
    };/*縺昴�ｮ譛医∪縺ｧ縺ｮ譌･謨ｰ縺ｮ蜥鯉ｼ育ｴｯ遨榊柱�ｼ�*/
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





