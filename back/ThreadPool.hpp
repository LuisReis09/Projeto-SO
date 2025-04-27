#ifndef _THREADPOOL_HPP_
#define _THREADPOOL_HPP_

    #include <iostream>
    #include <thread>
    #include <vector>
    #include <queue>
    #include <functional>
    #include <mutex>
    #include <condition_variable>
    #include <atomic>

    using namespace std;

    class ThreadPool {
        private:
            bool stop;
            vector<thread> threads;
            queue<function<void()>> tasks;
            mutex mtx;
            condition_variable cond_var;

        



        public:
            // Criam threads que ficam esperando por tarefas
            // Quando uma tarefa é adicionada, uma thread é notificada para processá-la
            ThreadPool(int num_threads) : stop(false) {
                for (int i = 0; i < num_threads; ++i) {
                    threads.emplace_back([this] {
                        while(true){
                            function<void()> task;

                            // Lock para pegar a tarefa da fila
                            {
                                unique_lock<mutex> lock(mtx);
                                cond_var.wait(lock, [this] { return stop || !tasks.empty(); });
                                if (stop && tasks.empty())
                                    return;
                                task = move(tasks.front());
                                tasks.pop();
                            }

                            // Executa a tarefa
                            task();
                        }
                    });
                }
            }

            ~ThreadPool() {
                stop_all_threads(); // Para todas as threads
            }

            void stop_all_threads() {
                {
                    lock_guard<mutex> lock(mtx);
                    stop = true;   // Indica que todas as threads devem parar.
                }
                cond_var.notify_all(); // Notifica todas as threads.
            
                // Agora, sem aguardar, "matar" todas as threads.
                for (auto& thread : threads) {
                    if (thread.joinable()) {
                        // Usando detach para que as threads terminem sem esperar pela execução.
                        thread.detach();
                    }
                }
            
                // Limpa a fila de tarefas para garantir que nenhuma tarefa incompleta continue
                tasks = queue<function<void()>>(); // Limpa a fila de tarefas.
                threads.clear(); // Limpa o vetor de threads.
            }

            // Adiciona uma tarefa a fila de tarefas
            void enqueue(function<void()> task) {
                // Adiciona a tarefa na fila
                lock_guard<mutex> lock(mtx);
                tasks.push(move(task));

                // Notifica uma thread para processar a tarefa
                cond_var.notify_one();
            }

            bool is_everything_done() {
                return tasks.empty() && stop;
            }
    };

#endif // _THREADPOOL_HPP_