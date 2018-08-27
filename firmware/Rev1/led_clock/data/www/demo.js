function mode_send(mode){
    target = document.getElementById("receive");
    target.innerText ="";
    
    var request = new XMLHttpRequest();
    
        request.onreadystatechange = function() {
            switch ( request.readyState){
                case 4:
                    target.innerText = "mode set ok";
                break;
            }
        
        };
    request.open("POST", "mode");
    request.send(mode);
}