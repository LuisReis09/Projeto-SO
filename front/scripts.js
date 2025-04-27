const twins = [...document.getElementsByClassName("twin")];
const color = {
    "Grayscale" : "gray_scale",
    "HSV" : "hsv",
    "RGB" : "rgb"
}
const thread_dd = document.getElementById("thread_dd");
const thread_sel = document.getElementById("selected_thread");
const filters = ["BLUR", "SHARPNESS", "SATURATION", "GRAYSCALE"];
const filter_dd = document.getElementById("filter_dd");
const filter_sel = document.getElementById("selected_filter");
const intensity = document.getElementById("intensity");
const process_but = document.getElementById("process");

const picker = document.getElementById("img-picker");
const input = document.getElementById("input-img");

// CAMPOS DE RESULTADOS
const single_thread_duration = document.getElementById("single-time");
const multi_thread_duration  = document.getElementById("multi-time");
const single_thread_image    = document.getElementById("single-out");
const multi_thread_image     = document.getElementById("mult-out");
const multi_thread_download      = document.getElementById("mult-out-download");
const single_thread_download     = document.getElementById("single-out-download");

var color_sel = "rgb";
var stopState = {
    mult : true,
    single : true,
}
var multi_thread_image_link = null;
var single_thread_image_link = null;

input.addEventListener("dragenter", dragenter, false);
input.addEventListener("dragover", dragover, false);
input.addEventListener("drop", drop, false);
process_but.addEventListener("click", process, false);
multi_thread_download.addEventListener("click", function() {
    multi_thread_download.href = multi_thread_image_link;
    multi_thread_download.download = "multithread.png";
    if(multi_thread_image_link == null) return;
    multi_thread_download.click();
});
single_thread_download.addEventListener("click", function() {
    single_thread_download.href = single_thread_image_link;
    single_thread_download.download = "singlethread.png";
    if(single_thread_image_link == null) return;
    single_thread_download.click();
});


twins.forEach((e) => {
    e.setAttribute("onclick", "toggle(this)");
});
intensity.setAttribute("onchange", "updateSel(this, selected_intensity)");

function toggle(e) {
    if(e.classList.contains("selected")) return;
    twins.forEach((el)=> {
        if(el === e) return;
        el.classList.remove("selected");
    })
    e.classList.add("selected");
    color_sel = color[e.innerHTML];
}

function fillDropdown(dd, opts, sel) {
    opts.forEach((e)=>{
        let newel = document.createElement("p");
        newel.innerHTML = e;
        newel.setAttribute("onclick", `selectDropdown(this, ${sel.id})`);
        dd.appendChild(newel);
    });
    sel.innerHTML = opts[0];
}

function selectDropdown(e, id) {
    id.innerHTML = e.innerHTML;
}

function updateSel(e, sel) {
    sel.innerHTML = e.value;
}

function updateImg(img) {
    let newsource = URL.createObjectURL(img);
    input.src = newsource;
}

function dragenter(e) {
    e.stopPropagation();
    e.preventDefault();
}
  
function dragover(e) {
    e.stopPropagation();
    e.preventDefault();
}
function drop(e) {
    e.stopPropagation();
    e.preventDefault();
  
    let dt = e.dataTransfer;
    let img = dt.files[0];
  
    updateImg(img);
}

function input_process() {
    if(stopState.mult && stopState.single) 
        process();
    else
        stopProcess();
}

function process(){
    let file = picker.files[0];
    let type = file.name.split(".")[1];
    /*
        rgb, gray_scale
        ^estou assumindo q hsv vai ser so "hsv"
    */
    // fetch(`/process?image=${file}&intensity=${intensity.value}&qtdThreads=${thread_sel.innerHTML}&filter=${filter_sel.innerHTML}&colorOption=${color_sel}`)

    const formData = new FormData();
    formData.append("image", file);
    formData.append("intensity", intensity.value);
    formData.append("qtdThreads", thread_sel.innerHTML);
    formData.append("filter", filter_sel.innerHTML);
    formData.append("colorOption", color_sel);
    formData.append("filetype", type);

    fetch("/process", {
        method: "POST",
        body: formData,
        
    })
    .then((response) => response.json())
    .then((data) => {
        stopState.mult = false;
        stopState.single = false;
        checkProcess();

        gettingImage_MultiThread(200);
        gettingImage_SingleThread(200);
    })
}


function checkProcess() {
    if(stopState.single && stopState.mult) {
        process_but.innerHTML = "Process";
        process_but.disabled = false;
    }else{
        process_but.innerHTML = "Await Processing...";
        process_but.disabled = true;
    }
}

window.onload = () => {
    fetch("/getThreadsOptions")
    .then((response) => response.json())
    .then((data) => {
        options = [];
        for(let i = 2; i <= Number(data.maxThreads); i++){
            options.push(i);
        }
        fillDropdown(thread_dd, options, thread_sel);
    })

    fetch("/getFiltersOptions")
    .then((response) => response.json())
    .then((data) => {
        fillDropdown(filter_dd, data.options, filter_sel);
    })
    
    intensity.value = 5;
};

function gettingImage_SingleThread(seconds) {
    // Essa funcao vai atualizando as imagens a cada x milissegundos, consultando o servidor para pegar uma versão atualizada
    // Isso mostra o progresso do processamento
    setTimeout(() => {
        fetch("/getSingleThreadImage")
        .then((response) => {
            const done = response.headers.get("done");
            const duration = response.headers.get("duration");
            return response.blob().then(blob => ({ blob, done, duration }));
        })
        .then(({blob, done, duration}) => {
            single_thread_image.src = URL.createObjectURL(blob);
            single_thread_image_link = single_thread_image.src;
            single_thread_duration.innerHTML = Number(duration).toLocaleString('de-DE', { minimumFractionDigits: 3, maximumFractionDigits: 3 }) + " s";
            
            if(done == 1) {
                stopState.single = true;
                checkProcess();
            }else{
                gettingImage_SingleThread(seconds);
            }
        })
    }, seconds);
}

function gettingImage_MultiThread(seconds) {
    // Essa funcao vai atualizando as imagens a cada x milissegundos, consultando o servidor para pegar uma versão atualizada
    // Isso mostra o progresso do processamento
    setTimeout(() => {
        fetch("/getMultiThreadImage")
        .then((response) => {
            const done = response.headers.get("done");
            const duration = response.headers.get("duration");
            return response.blob().then(blob => ({ blob, done, duration }));
        })
        .then(({blob, done, duration}) => {
            multi_thread_image.src = URL.createObjectURL(blob);
            multi_thread_image_link = multi_thread_image.src;
            multi_thread_duration.innerHTML = Number(duration).toLocaleString('de-DE', { minimumFractionDigits: 3, maximumFractionDigits: 3 }) + " s";
            if(done == 1) {
                stopState.mult = true;
                checkProcess();
            }else{
                gettingImage_MultiThread(seconds);
            }
        })
    }, seconds);
}