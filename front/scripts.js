// ELEMENTOS RELEVANTES
const twins = [...document.getElementsByClassName("twin")];

// Objeto com as relações das opções de cores e os botões correspondentes
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

// EVENTOS

// Para arrastar arquivso a para dentro do input
input.addEventListener("dragenter", dragenter, false);
input.addEventListener("dragover", dragover, false);
input.addEventListener("drop", drop, false);

// Botão de processar
process_but.addEventListener("click", process, false);

// Botões de download dos outputs
multi_thread_download.addEventListener("click", function() {
    if(multi_thread_image_link == null) return;
    multi_thread_download.href = multi_thread_image_link;
    multi_thread_download.download = "multithread.png";
});
single_thread_download.addEventListener("click", function() {
    if(single_thread_image_link == null) return;
    single_thread_download.href = single_thread_image_link;
    single_thread_download.download = "singlethread.png";
});


// Toggle para os botões de cores
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

// Função para preencher o dropdown das opções de thread e filtros
// Chamada com uma lista qualquer de opções para um elemento de dropdown
function fillDropdown(dd, opts, sel) {
    opts.forEach((e)=>{
        let newel = document.createElement("p");
        newel.innerHTML = e;
        newel.setAttribute("onclick", `selectDropdown(this, ${sel.id})`);
        dd.appendChild(newel);
    });
    sel.innerHTML = opts[0];
}
document.querySelectorAll(".dropbtn").forEach((btn) => {
    btn.addEventListener("click", () => {
        const dropdown = btn.nextElementSibling;
        dropdown.classList.toggle("hidden");
    });
});

// Função para selecionar a opção do dropdown
function selectDropdown(e, id) {
    id.innerHTML = e.innerHTML;

    e.parentElement.classList.add("hidden");
}

function updateSel(e, sel) {
    sel.innerHTML = e.value;
}

// Função para atualizar a imagem de entrada
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

// Função para começar ou parar o processamento baseado no estado atual
function input_process() {
    if(stopState.mult && stopState.single) 
        process();
    else
        stopProcess();
}

// Função que chama a filtragem em si
function process(){
    let file = picker.files[0];
    let type = file.name.split(".")[1];

    // Junta os dados da imagem e das opções escolhidas em um FormData para enviar para o backend
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
        // Coloca o estado de processamento como false para que o botão de processar fique desabilitado
        stopState.mult = false;
        stopState.single = false;
        checkProcess();

        gettingImage_MultiThread(200);
        gettingImage_SingleThread(200);
    })
}

// Confere o estado atual de processamento e habilita ou desabilita o botão de processar
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
    // Preenche os dropdowns de threads e filtros com as opções disponíveis informadas pelo backend
    // Este trecho é executado quando a página é carregada
    // Isso é feito para que o front se adapte a qualquer mudança nas funções abrangidas pelo backend
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
            // Aqui, a imagem é atualizada conforme o processamento avança e o tempo de processamento corrente é mostrado
            single_thread_image.src = URL.createObjectURL(blob);
            single_thread_image_link = single_thread_image.src;
            single_thread_duration.innerHTML = Number(duration).toLocaleString('de-DE', { minimumFractionDigits: 1, maximumFractionDigits: 1 }) + " ms";
            
            // Se o processamento estiver completo, a função de atualização é parada e o estado de processamento para a imagem singlethread é atualizado
            // Tão como a imagem em si
            if(done == "1") {
                stopState.single = true;
                checkProcess();
            }else{
                gettingImage_SingleThread(seconds);
            }
        })
    }, seconds);
}

function gettingImage_MultiThread(seconds) {
    // O mesmo que a função acima, mas para o processamento multithread
    setTimeout(() => {
        fetch("/getMultiThreadImage")
        .then((response) => {
            const done = response.headers.get("done");
            const duration = response.headers.get("duration");
            return response.blob().then(blob => ({ blob, done, duration }));
        })
        .then(({blob, done, duration}) => {
            // Update da imagem e do tempo de processamento
            multi_thread_image.src = URL.createObjectURL(blob);
            multi_thread_image_link = multi_thread_image.src;
            multi_thread_duration.innerHTML = Number(duration).toLocaleString('de-DE', { minimumFractionDigits: 1, maximumFractionDigits: 1 }) + " ms";
            
            // Se o processamento estiver completo, a função de atualização é parada e o estado de processamento para a imagem multithread é atualizado
            // Tão como a imagem em si
            if(done == "1") {
                stopState.mult = true;
                checkProcess();
            }else{
                gettingImage_MultiThread(seconds);
            }
        })
    }, seconds);
}