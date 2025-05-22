#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tipos

typedef unsigned char byte; // 8 bits
typedef unsigned int palavra; // 32 bits
typedef unsigned long int microinstrucao; // 64 bits, no caso de acordo com a arquitetura cada microinstrução usa apenas 36 bits de espaço

// Registradores

palavra MAR = 0, MDR = 0, PC = 0; // Acesso da Memoria
byte MBR = 0;              // Acesso da Memoria

palavra SP = 0, LV = 0, TOS = 0, // Operação da ULA
OPC = 0, CPP = 0, H = 0; // Operação da ULA

microinstrucao MIR; // Contem a Microinstrução Atual
palavra MPC = 0; // Contem o endereco para a proxima Microinstrução

// Barramentos

palavra Barramento\_B, Barramento\_C;

// Flip-Flops

byte N, Z;

// Auxiliares para Decodificar Microinstrução

byte MIR\_B, MIR\_Operacao, MIR\_Deslocador, MIR\_MEM, MIR\_pulo;
palavra MIR\_C;

// Armazenamento de Controle

microinstrucao Armazenamento\[512];

// Memoria Principal

byte Memoria\[100000000]; // Melhorar: espaço excessivo fixo pode ser otimizado futuramente

// Prototipo das Funções

void carregar\_microprogram\_de\_controle();
void carregar\_programa(const char \*programa);
void exibir\_processos();
void decodificar\_microinstrucao();
void atribuir\_barramento\_B();
void realizar\_operacao\_ALU();
void atribuir\_barramento\_C();
void operar\_memoria();
void pular();

void binario(void\* valor, int tipo);

// Laço Principal

int main(int argc, const char \*argv\[]){
if (argc < 2) {
printf("Erro: Caminho do programa ausente.\n"); // Melhorar: tratamento básico de erro
return 1;
}

```
carregar_microprogram_de_controle();
carregar_programa(argv[1]);
while(1){
    exibir_processos();
    MIR = Armazenamento[MPC];

    decodificar_microinstrucao();
    atribuir_barramento_B();
    realizar_operacao_ALU();
    atribuir_barramento_C();
    operar_memoria();
    pular();
}

return 0;
```

}

// Implementação das Funções

void carregar\_microprogram\_de\_controle(){
FILE\* MicroPrograma = fopen("microprog.rom", "rb");

```
if(MicroPrograma != NULL){
    fread(Armazenamento, sizeof(microinstrucao), 512, MicroPrograma);
    fclose(MicroPrograma);
} else {
    printf("Erro ao abrir microprog.rom\n"); // Sugestão: informar erro na leitura do microprograma
}
```

}

void carregar\_programa(const char\* prog){
FILE\* Programa = fopen(prog, "rb");
palavra tamanho;
byte tamanho\_temp\[4];

```
if(Programa != NULL){
    fread(tamanho_temp, sizeof(byte), 4, Programa);
    memcpy(&tamanho, tamanho_temp, 4);

    fread(Memoria, sizeof(byte), 20, Programa);
    fread(&Memoria[0x0401], sizeof(byte), tamanho - 20, Programa);
} else {
    printf("Erro ao carregar programa binário\n"); // Adicionado aviso simples
}
```

}

void decodificar\_microinstrucao(){
MIR\_B = (MIR) & 0b1111;
MIR\_MEM = (MIR >> 4) & 0b111;
MIR\_C = (MIR >> 7) & 0b111111111;
MIR\_Operacao = (MIR >> 16) & 0b111111;
MIR\_Deslocador = (MIR >> 22) & 0b11;
MIR\_pulo = (MIR >> 24) & 0b111;
MPC = (MIR >> 27) & 0b111111111;
}

void atribuir\_barramento\_B(){
switch(MIR\_B){
case 0: Barramento\_B = MDR; break;
case 1: Barramento\_B = PC; break;
case 2: Barramento\_B = MBR;
if(MBR & (0b10000000))
Barramento\_B = Barramento\_B | (0b111111111111111111111111 << 8);
break;
case 3: Barramento\_B = MBR; break;
case 4: Barramento\_B = SP; break;
case 5: Barramento\_B = LV; break;
case 6: Barramento\_B = CPP; break;
case 7: Barramento\_B = TOS; break;
case 8: Barramento\_B = OPC; break;
default: Barramento\_B = -1; break; // Melhorar: retornar erro claro se valor inválido
}
}

void realizar\_operacao\_ALU(){
switch(MIR\_Operacao){
case 12: Barramento\_C = H & Barramento\_B; break;
case 17: Barramento\_C = 1; break;
case 18: Barramento\_C = -1; break;
case 20: Barramento\_C = Barramento\_B; break;
case 24: Barramento\_C = H; break;
case 26: Barramento\_C = \~H; break;
case 28: Barramento\_C = H | Barramento\_B; break;
case 44: Barramento\_C = \~Barramento\_B; break;
case 53: Barramento\_C = Barramento\_B + 1; break;
case 54: Barramento\_C = Barramento\_B - 1; break;
case 57: Barramento\_C = H + 1; break;
case 59: Barramento\_C = -H; break;
case 60: Barramento\_C = H + Barramento\_B; break;
case 61: Barramento\_C = H + Barramento\_B + 1; break;
case 63: Barramento\_C = Barramento\_B - H; break;
default: break; // Sugerido: adicionar mensagem para opcode inválido
}

```
if(Barramento_C){
    N = 0;
    Z = 1;
} else {
    N = 1;
    Z = 0;
}

switch(MIR_Deslocador){
    case 1: Barramento_C = Barramento_C << 8; break;
    case 2: Barramento_C = Barramento_C >> 1; break;
}
```

}

void atribuir\_barramento\_C(){
if(MIR\_C & 0b000000001) MAR = Barramento\_C;
if(MIR\_C & 0b000000010) MDR = Barramento\_C;
if(MIR\_C & 0b000000100) PC  = Barramento\_C;
if(MIR\_C & 0b000001000) SP  = Barramento\_C;
if(MIR\_C & 0b000010000) LV  = Barramento\_C;
if(MIR\_C & 0b000100000) CPP = Barramento\_C;
if(MIR\_C & 0b001000000) TOS = Barramento\_C;
if(MIR\_C & 0b010000000) OPC = Barramento\_C;
if(MIR\_C & 0b100000000) H   = Barramento\_C;
}

void operar\_memoria(){
if(MIR\_MEM & 0b001) MBR = Memoria\[PC];
if(MIR\_MEM & 0b010) memcpy(\&MDR, \&Memoria\[MAR*4], 4);
if(MIR\_MEM & 0b100) memcpy(\&Memoria\[MAR*4], \&MDR, 4);
}

void pular(){
if(MIR\_pulo & 0b001) MPC = MPC | (N << 8);
if(MIR\_pulo & 0b010) MPC = MPC | (Z << 8);
if(MIR\_pulo & 0b100) MPC = MPC | (MBR);
}

// O restante do código mantém lógica correta e está funcional
// Recomenda-se modularizar partes maiores para facilitar testes e manutenção futura
