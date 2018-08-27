function light_set(){
    target = document.getElementById("output");
    target.innerText ="";

    var str_light = document.forms.form1.light.value;

    if(Number(str_light)>255){
        target.innerText = "max over";
        return;
    }else if(str_light==""){
        target.innerText = "blank"; 
        return;     
    }else if(Number(str_light)<0){
        target.innerText = "less than 0"; 
        return;     
    }

    var json ={};
    json.light = str_light;

    var request = new XMLHttpRequest();
    
        request.onreadystatechange = function() {
            switch ( request.readyState){
                case 4:
                    target.innerText = "setting ok";
                break;
            }
        
        };
    
    
        request.open("POST", "light");
        request.setRequestHeader("Content-Type", "application/json");
        request.responseType = "json";
        request.send(JSON.stringify(json));

}