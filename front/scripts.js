const twins = [...document.getElementsByClassName("twin")];
const thread_num = [2, 4, 8, 16, 32, 64, 128];
const thread_dd = document.getElementById("thread_dd");
const thread_sel = document.getElementById("selected_thread");
const filters = ["BLUR", "SHARPNESS", "SATURATION", "GRAYSCALE"];
const filter_dd = document.getElementById("filter_dd");
const filter_sel = document.getElementById("selected_filter");

twins.forEach((e) => {
    e.setAttribute("onclick", "toggle(this)");
});

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

window.onload = () => {
    fillDropdown(thread_dd, thread_num, thread_sel);
    fillDropdown(filter_dd, filters, filter_sel);
};