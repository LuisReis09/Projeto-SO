#ifndef _THREADPOOL_HPP_
#define _THREADPOOL_HPP_

#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <pthread.h>  // Incluir a biblioteca pthread

using namespace std;

/*
    * Classe ThreadPool
    *
    * Esta classe implementa um pool de threads que pode ser usado para executar tarefas em paralelo.
    * O pool de threads é inicializado com um número fixo de threads, e as tarefas podem ser adicionadas a uma fila.
    * As threads retiram as tarefas da fila e as executam. O pool pode ser parado quando não for mais necessário.
*/
class ThreadPool {
    private:

        /**
         * Variável para indicar se as threads devem parar ou não.
         *
         * - `true`: as threads devem parar de processar tarefas e sair. 
         * - `false`: as threads continuam processando tarefas normalmente.
        */
        bool stop;

        /*
         * Vetor de threads que compõem o pool de threads.
         *
         * Cada thread no vetor é uma instância da classe `pthread_t`, que representa uma thread do sistema operacional.
        */
        vector<pthread_t> threads;

        /*
         * Fila de tarefas a serem executadas pelas threads do pool.
         *
         * Cada tarefa é uma função que não recebe parâmetros e não retorna valor (`function<void()>`).
         * As threads retiram tarefas dessa fila para processá-las.
        */
        queue<function<void()>> tasks;

        /*
         * Mutex para proteger o acesso à fila de tarefas.
         *
         * Garante que apenas uma thread possa acessar a fila de tarefas ao mesmo tempo, evitando condições de corrida.
        */
        pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

        /*
         * Variável de condição para notificar as threads quando uma nova tarefa é adicionada à fila.
         *
         * As threads aguardam nessa variável de condição até que uma nova tarefa esteja disponível.
        */
        pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;


        /*
         * Função que será executada por cada thread do pool.
         *
         * Essa função fica em loop, aguardando novas tarefas na fila. Quando uma tarefa é adicionada, a thread a executa.
         * Se `stop` for verdadeiro e a fila estiver vazia, a thread sai do loop e termina sua execução
         * 
         * @param arg Ponteiro para o objeto `ThreadPool` que contém a fila de tarefas e outras informações necessárias.
         * @return `nullptr` quando a thread termina sua execução
        */
        static void* thread_function(void* arg) {
            // Converte o argumento recebido para um ponteiro do tipo ThreadPool
            ThreadPool* pool = static_cast<ThreadPool*>(arg);

            // Loop infinito para processar tarefas
            while(true) {
                // Variável para armazenar a tarefa a ser executada
                function<void()> task;

                // Lock para pegar a tarefa da fila
                pthread_mutex_lock(&pool->mtx);

                // Enquanto não houver tarefas e stop for falso, espera pela notificação
                while (!pool->stop && pool->tasks.empty()) {
                    pthread_cond_wait(&pool->cond_var, &pool->mtx);
                }

                // Se stop for verdadeiro e a fila estiver vazia, sai da thread
                if (pool->stop && pool->tasks.empty()) {
                    pthread_mutex_unlock(&pool->mtx);
                    return nullptr;
                }

                // Se houver tarefas, pega a primeira tarefa da fila, removendo-a da fila
                task = move(pool->tasks.front());
                pool->tasks.pop();

                // Libera o lock para permitir que outras threads acessem a fila
                pthread_mutex_unlock(&pool->mtx);

                // Executa a tarefa
                task();
            }

            // Retorna nullptr quando a thread termina
            return nullptr;
        }

    public:
        /*  
         * Construtor da classe ThreadPool.
         *
         * Inicializa o pool de threads com o número especificado de threads.
         * Cada thread é criada e adicionada ao vetor de threads.
         *
         * @param num_threads Número de threads a serem criadas no pool
        
        */
        ThreadPool(int num_threads) : stop(false) {

            for (int i = 0; i < num_threads; ++i) {
                // Inicializa o vetor de threads com o número de threads especificado
                pthread_t thread;

                // Criando threads com pthread_create
                if (pthread_create(&thread, nullptr, thread_function, this) != 0) {
                    cerr << "Erro ao criar thread!" << endl;
                    exit(1);
                }

                // Adiciona a thread criada ao vetor de threads
                threads.push_back(thread);
            }
        }

        /*
         * Destrutor da classe ThreadPool.
         *
         * Para todas as threads do pool, chama a função `stop_all_threads()` para parar e limpar as threads.
        */
        ~ThreadPool() {
            stop_all_threads();
        }

        /*
         * Para todas as threads do pool, chama a função `stop_all_threads()` para parar e limpar as threads.
         *
         * Essa função é chamada no destrutor da classe ThreadPool para garantir que todas as threads sejam paradas corretamente.
        */
        void stop_all_threads() {

            // Lock para garantir que apenas uma thread acesse a variável stop ao mesmo tempo
            pthread_mutex_lock(&mtx);

            // Define a variável stop como verdadeira, indicando que as threads devem parar
            stop = true;

            // Libera o lock para permitir que outras threads acessem a variável stop
            pthread_mutex_unlock(&mtx);

            // Notifica todas as threads para que elas possam sair do loop de espera
            pthread_cond_broadcast(&cond_var);

            // Espera que todas as threads terminem sua execução
            for (auto& thread : threads) {
                pthread_join(thread, nullptr);
            }

            // Limpa a fila de tarefas para garantir que nenhuma tarefa incompleta continue
            while (!tasks.empty()) {
                tasks.pop();
            }

            // Limpa o vetor de threads
            threads.clear();
        }

        /*
         * Adiciona uma nova tarefa à fila de tarefas do pool.
         *
         * Essa função é chamada para adicionar uma nova tarefa que será executada por uma das threads do pool.
         *
         * @param task A tarefa a ser adicionada à fila. Deve ser uma função que não recebe parâmetros e não retorna valor.
        */
        void enqueue(function<void()> task) {
            // Lock para garantir que apenas uma thread acesse a fila de tarefas ao mesmo tempo
            pthread_mutex_lock(&mtx);

            // Adiciona a nova tarefa à fila de tarefas
            tasks.push(move(task));

            // Notifica uma thread para processar a tarefa
            pthread_cond_signal(&cond_var);
            pthread_mutex_unlock(&mtx);
        }

        /*
         * Verifica se todas as tarefas foram concluídas e se o pool de threads deve parar.
         *
         * Essa função é chamada para verificar se todas as tarefas foram concluídas e se o pool de threads deve parar.
         *
         * @return true se todas as tarefas foram concluídas e o pool deve parar, false caso contrário.
        */
        bool is_everything_done() {
            pthread_mutex_lock(&mtx);
            bool result = tasks.empty() && stop;
            pthread_mutex_unlock(&mtx);
            return result;
        }
};

#endif // _THREADPOOL_HPP_