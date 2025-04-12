const twins = [...document.getElementsByClassName("twin")];
const thread_num = [2, 4, 8, 16, 32, 64, 128];
const thread_dd = document.getElementById("thread_dd");
const thread_sel = document.getElementById("selected_thread");
const filters = ["BLUR", "SHARPNESS", "SATURATION", "GRAYSCALE"];
const filter_dd = document.getElementById("filter_dd");
const filter_sel = document.getElementById("selected_filter");
const intensity = document.getElementById("intensity");

const picker = document.getElementById("img-picker");
const input = document.getElementById("input-img");

input.addEventListener("dragenter", dragenter, false);
input.addEventListener("dragover", dragover, false);
input.addEventListener("drop", drop, false);

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
    console.log(newsource);
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
  

window.onload = () => {
    fillDropdown(thread_dd, thread_num, thread_sel);
    fillDropdown(filter_dd, filters, filter_sel);
    intensity.value = 70;
};