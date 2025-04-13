const twins = [...document.getElementsByClassName("twin")];
const color = {
    "Grayscale" : "gray_scale",
    "Colored" : "rgb"
}
const thread_dd = document.getElementById("thread_dd");
const thread_sel = document.getElementById("selected_thread");
const filters = ["BLUR", "SHARPNESS", "SATURATION", "GRAYSCALE"];
const filter_dd = document.getElementById("filter_dd");
const filter_sel = document.getElementById("selected_filter");
const intensity = document.getElementById("intensity");

const picker = document.getElementById("img-picker");
const input = document.getElementById("input-img");

var color_sel = "rgb";

input.addEventListener("dragenter", dragenter, false);
input.addEventListener("dragover", dragover, false);
input.addEventListener("drop", drop, false);

document.getElementById("process").addEventListener("click", process, false);

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

function process(){
    let file = picker.files[0];
    let type = file.name.split(".")[1];
    /*
        rgb, gray_scale
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
        console.log(data);
    })
}
  

window.onload = () => {
    fetch("/getThreadsOptions")
    .then((response) => response.json())
    .then((data) => {
        fillDropdown(thread_dd, data.options, thread_sel);
    })
    
    fillDropdown(filter_dd, filters, filter_sel);
    intensity.value = 5;
};