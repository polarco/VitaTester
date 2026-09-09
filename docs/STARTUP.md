# Startup correction candidate — 1.5.1 / Correção de inicialização

## Observed / Observado

The user reported immediate return to LiveArea after launching 1.5.0. FTP readback
confirmed installed `eboot.bin` and SFO match the release exactly. The stress log
directory did not exist. No dumps were read. This does not yet distinguish every
possible cause of launch failure.

O usuário relatou retorno imediato à LiveArea ao abrir a 1.5.0. Executável e SFO
instalados conferem com a release; a pasta do log não existia. Nenhum dump foi lido.
Ainda não é possível excluir outras causas de falha de abertura.

## Confirmed defect / Defeito confirmado

Capture/logger used affinity `0x7`; workers used `1/2/4`. The pinned SDK declares
user CPU masks as `0x10000/0x20000/0x40000`, with union `0x70000`. The startup code
exited silently if capture creation failed; the host mocks accepted any affinity
and therefore missed the mismatch. The code now uses SDK macros and preserves
original initialization return values. Host mocks reject the previous masks.

Captura/logger usavam `0x7`; workers usavam `1/2/4`. O SDK fixado define máscaras
userland como `0x10000/0x20000/0x40000`, união `0x70000`. A inicialização encerrava
sem mensagem quando a captura falhava; mocks aceitavam qualquer máscara. Agora
as macros do SDK são usadas, os códigos originais são preservados e os mocks
rejeitam os valores anteriores.

Source: [VitaSDK CPU masks](https://docs.vitasdk.org/group__SceCpuUser.html) and
[thread creation contract](https://docs.vitasdk.org/kernel_2threadmgr_2thread_8h_source.html),
also verified in the project's pinned SDK container.

## Next physical check / Próximo teste físico

Install `VitaTester-1.5.1.vpk` through VitaShell, replacing the existing bubble.
Keep stress off. Launch and confirm the three-card menu, then enter Input Test.
If a startup error appears, record its stage/code. If it returns to LiveArea,
reopen FTP so `ux0:data/VitaTester/startup.txt` can be read. That append-only
file contains version, stage and return code; it records no raw input or identifiers.
Failure to write the file does not itself abort startup. A failure before `main`
can still leave no record. A font/graphics failure is logged when storage permits.

Instale `VitaTester-1.5.1.vpk` pelo VitaShell, substituindo a bolha. Mantenha stress
desligado. Confirme o menu com três cartões e entre no Input Test. Se aparecer
falha, informe etapa/código. Se voltar à LiveArea, reabra FTP para leitura de
`ux0:data/VitaTester/startup.txt`: arquivo acrescentado entre sessões, com versão,
etapa e retorno, sem entradas brutas ou identificadores. Falha de escrita não
aborta a abertura. Falha anterior ao `main` pode continuar sem registro; falhas
de fonte/gráficos são registradas quando o armazenamento permite.

A successful launch only closes this startup gate. Stress, Scanner and lifecycle
validation remain pending. Published releases 1.4.0 and 1.5.0 are preserved.
Abrir com sucesso encerra apenas o portão de inicialização. Stress, Scanner e
ciclo de vida continuam pendentes; releases 1.4.0 e 1.5.0 preservadas.
