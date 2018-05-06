function timeset(){
    target = document.getElementById("output");
    target.innerText ="";
    var str_year = document.forms.form1.year.value;
    var str_month = document.forms.form1.month.value;
      
        if(Number(str_month)>12){
            target.innerText = "too many month";
            return;
        }else if(str_month==""){
            target.innerText = "month blank"; 
            return;            
        }else if(str_month=="0"){
            target.innerText = "too small month"; 
        }

    var str_day = document.forms.form1.day.value;

        if(Number(str_day)>31){
            target.innerText = "too many day";
            return;
        }else if(str_day==""){
            target.innerText = "day blank"; 
            return;     
        }
    var str_hour = document.forms.form1.hour.value;

        if(Number(str_hour)>23){
            target.innerText = "too many hour";
            return;
        }else if(str_hour==""){
            target.innerText = "hour blank"; 
            return;     
        }
    var str_min = document.forms.form1.min.value;

        if(Number(str_min)>59){
            target.innerText = "too many min";
            return;
        }else if(str_min == ""){
            target.innerText = "min blank"; 
            return; 
        }
    var str_sec = document.forms.form1.sec.value;

        if(Number(str_sec)>59){
            target.innerText = "too many sec";
            return;
        }else if(str_sec == ""){
            target.innerText = "sec blank"; 
            return; 
        }
    //var str_send = {"year":str_year,"month":str_month,"day":str_day,"hour":str_hour,"min":str_min,"sec":str_sec};
    var json ={};
    json.year = str_year;
    json.month = str_month;
    json.day = str_day;
    json.hour = str_hour;
    json.min = str_min;
    json.sec = str_sec;

    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        switch ( request.readyState){
            case 4:
                target.innerText = "setting ok";
            break;
        }
    
    };


    request.open("POST", "setting");
    request.setRequestHeader("Content-Type", "application/json");
    request.responseType = "json";
    request.send(JSON.stringify(json));
}

function system_timeset(){
    target = document.getElementById("output");
    var sys_now = new Date();
    var json ={};
    json.year = sys_now.getFullYear();
    json.month = sys_now.getMonth()+1;
    json.day = sys_now.getDate();
    json.hour = sys_now.getHours();
    json.min = sys_now.getMinutes();
    json.sec = sys_now.getSeconds();

    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        switch ( request.readyState){
            case 4:
                target.innerText = "system time setting ok";
            break;
        }
    
    };


    request.open("POST", "setting");
    request.setRequestHeader("Content-Type", "application/json");
    request.responseType = "json";
    request.send(JSON.stringify(json));

}