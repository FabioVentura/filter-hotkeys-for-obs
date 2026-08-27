# Filter Hotkeys for OBS

**Author / Autor:** Fabio Ventura  
**Email:** [streamprogamers@proton.me](mailto:streamprogamers@proton.me)  
**Version / Versao:** 1.0.0  
**License / Licenca:** GNU General Public License v2 or later

---

## English

### Overview

Filter Hotkeys for OBS is a native OBS Studio filter that lets you select another filter on the same source and control it with a keyboard shortcut. Each controller instance can use a different target filter and a different shortcut. The available actions are **toggle**, **enable**, and **disable**.

The controller is implemented as a normal video filter. It passes the video through without changing the image and only changes the enabled state of the selected target filter when its registered hotkey is pressed.

### Download the Windows installer

Download the latest Windows x64 installer from the [Releases page](https://github.com/FabioVentura/filter-hotkeys-for-obs/releases). Download the ZIP asset, extract it, close OBS Studio, and run `Install-FilterHotkeys.cmd`. The installer places the plugin in the OBS installation and keeps a manifest for safe removal. The release asset is intended for OBS Studio 32.2.2 x64 and compatible 64-bit versions.

The repository includes a GitHub Actions workflow named **Windows Installer Release**. It can be started from the **Actions** tab with **Run workflow**, or automatically by pushing a tag named `installer-v*`, such as `installer-v1.0.9`. The workflow builds the plugin, creates the installer ZIP, uploads it as an artifact, and attaches it to the tag release.

### Features

- Select a target filter from the filters attached to the same source.
- Toggle the target filter state.
- Force the target filter on.
- Force the target filter off.
- Create multiple controller instances with independent shortcuts.
- Accept shortcuts such as `F9`, `Ctrl+F9`, `Alt+F10`, and `Ctrl+Shift+F11`.
- Keep the user-facing configuration inside the OBS filter properties.
- Use the OBS frontend hotkey registration mechanism so the controller works correctly even though OBS filters are private source objects.

### Requirements

- OBS Studio 32.2.2 or a compatible 64-bit OBS Studio installation.
- A compiler and CMake version supported by the OBS plugin template.
- The OBS development dependencies required by the selected platform.

### Build on Linux

Install the OBS development packages and the build tools required by the OBS plugin template. From the project directory, run:

```bash
cmake --preset ubuntu-x86_64
cmake --build --preset ubuntu-x86_64
cmake --install build_x86_64
```

The exact dependency package names depend on the Linux distribution. The project uses the OBS plugin-template CMake layout and the `ubuntu-x86_64` preset.

### Build on Windows x64

Open a **Developer Command Prompt for Visual Studio** with CMake, MSVC, the Windows SDK, and the OBS development dependencies available. From the project directory, run:

```powershell
cmake --preset windows-x64 -DOBS_INSTALL_PREFIX="C:/path/to/obs-sdk-or-install-prefix"
cmake --build --preset windows-x64 --config RelWithDebInfo
cmake --install build_x64 --config RelWithDebInfo
```

`OBS_INSTALL_PREFIX` is optional when `libobs` is already discoverable through `CMAKE_PREFIX_PATH`. It should point to the OBS development or installation prefix containing the CMake package files for `libobs`. The resulting module is installed under the OBS plugin directory as `filter-hotkeys-for-obs.dll`.

### Build on macOS

On a macOS system with Xcode and the OBS development dependencies installed, run:

```bash
cmake --preset macos
cmake --build --preset macos --config RelWithDebInfo
cmake --install build_macos --config RelWithDebInfo
```

Code signing may be required when installing into a signed OBS application bundle.

### Manual installation layout

Close OBS before copying files. Use the platform-specific layout below:

| Platform | Module | Locale data |
|---|---|---|
| Windows x64 | `obs-plugins\\64bit\\filter-hotkeys-for-obs.dll` | `data\\obs-plugins\\filter-hotkeys-for-obs\\locale` |
| Linux x86_64 | `lib/x86_64-linux-gnu/obs-plugins/filter-hotkeys-for-obs.so` | `share/obs/obs-plugins/filter-hotkeys-for-obs/locale` |
| macOS | `filter-hotkeys-for-obs.plugin` | Inside the plugin bundle |

Restart OBS after installation. The filter is available when adding a filter to a source under the name **Filter Hotkeys - Filter Controller** or its translated equivalent.

### Configuration

1. Add the filter that you want to control to a source.
2. Add **Filter Hotkeys - Filter Controller** to the same source.
3. Open the controller properties.
4. Select the target filter from the list.
5. Choose one action: **Toggle state**, **Enable filter**, or **Disable filter**.
6. Enter a shortcut such as `F9` or `Ctrl+F9`.
7. Click **Save binding**.
8. Click **Test action now** to verify the target filter and the selected action.
9. Click **OK** and press the same shortcut while the OBS main window is focused.

Multiple controller instances can be added to the same source. Use a different target and shortcut for each instance when independent control is required.

### Accepted shortcut format

The parser accepts a key name with optional modifiers separated by `+`:

```text
F1 through F24
A through Z
0 through 9
Space
Enter
Escape
Ctrl+F9
Alt+F10
Ctrl+Shift+F11
```

The parser also accepts OBS internal names such as `OBS_KEY_F9` when needed. Modifier aliases include `Ctrl`/`Control`, `Alt`, `Shift`, and `Win`/`Meta`/`Cmd`.

### Troubleshooting

If the controller does not appear, close OBS and confirm that the module architecture matches the OBS architecture. OBS Studio 64-bit requires a 64-bit plugin.

If the target list is empty, add the target filter first, save the source, close the properties window, and open the controller properties again.

If **Test action now** changes the target filter but the keyboard shortcut does not, check the OBS log. A successful configuration should contain messages similar to:

```text
Filter Hotkeys loaded successfully
Filter Hotkeys: binding applied 'F9' -> OBS_KEY_F9
Filter Hotkeys: callback triggered
```

The controller uses a frontend hotkey registration mechanism because OBS filters are private source objects. Do not expect the shortcut to appear as a regular source hotkey tied to the filter object itself.

### License and contact

This project is distributed under the GNU General Public License version 2 or later. For questions, bug reports, or contributions, contact Fabio Ventura at [streamprogamers@proton.me](mailto:streamprogamers@proton.me).

---

## Portugues (Brasil)

### Visao geral

O Filter Hotkeys para OBS e um filtro nativo que permite selecionar outro filtro da mesma fonte e controla-lo por uma tecla de atalho. Cada instancia do controlador pode usar um filtro-alvo e uma tecla diferentes. As acoes disponiveis sao **alternar**, **ativar** e **desativar**.

O controlador e implementado como um filtro de video comum. Ele repassa o video sem alterar a imagem e somente modifica o estado de habilitacao do filtro escolhido quando a hotkey registrada e pressionada.

### Download do instalador Windows

Baixe o instalador Windows x64 mais recente na pagina de [Releases](https://github.com/FabioVentura/filter-hotkeys-for-obs/releases). Baixe o arquivo ZIP, extraia-o, feche o OBS Studio e execute `Install-FilterHotkeys.cmd`. O instalador coloca o plugin na instalacao do OBS e mantem um manifesto para permitir a remocao segura. O arquivo da release foi preparado para o OBS Studio 32.2.2 x64 e versoes compativeis de 64 bits.

O repositorio inclui um workflow do GitHub Actions chamado **Windows Installer Release**. Ele pode ser iniciado na aba **Actions** com **Run workflow**, ou automaticamente ao enviar uma tag com o nome `installer-v*`, como `installer-v1.0.9`. O workflow compila o plugin, cria o ZIP do instalador, publica o arquivo como artifact e anexa o ZIP a release da tag.

### Recursos

- Selecionar um filtro-alvo entre os filtros anexados a mesma fonte.
- Alternar o estado do filtro-alvo.
- Forcar a ativacao do filtro.
- Forcar a desativacao do filtro.
- Criar varias instancias com teclas independentes.
- Aceitar teclas como `F9`, `Ctrl+F9`, `Alt+F10` e `Ctrl+Shift+F11`.
- Manter a configuracao dentro das propriedades do filtro do OBS.
- Usar o mecanismo de registro de hotkeys frontend do OBS, que funciona corretamente mesmo que filtros sejam objetos privados de fonte.

### Requisitos

- OBS Studio 32.2.2 ou uma instalacao compativel do OBS Studio 64-bit.
- Compilador e CMake compativeis com o template de plugins do OBS.
- As dependencias de desenvolvimento do OBS exigidas pela plataforma escolhida.

### Compilacao no Linux

Instale os pacotes de desenvolvimento do OBS e as ferramentas de compilacao exigidas pelo template de plugins do OBS. Na pasta do projeto, execute:

```bash
cmake --preset ubuntu-x86_64
cmake --build --preset ubuntu-x86_64
cmake --install build_x86_64
```

Os nomes exatos dos pacotes dependem da distribuicao Linux. O projeto usa a estrutura CMake do template de plugins do OBS e o preset `ubuntu-x86_64`.

### Compilacao no Windows x64

Abra um **Developer Command Prompt for Visual Studio** com CMake, MSVC, Windows SDK e as dependencias de desenvolvimento do OBS disponiveis. Na pasta do projeto, execute:

```powershell
cmake --preset windows-x64 -DOBS_INSTALL_PREFIX="C:/caminho/do/obs-sdk-ou-prefixo-de-instalacao"
cmake --build --preset windows-x64 --config RelWithDebInfo
cmake --install build_x64 --config RelWithDebInfo
```

`OBS_INSTALL_PREFIX` e opcional quando o `libobs` ja pode ser encontrado pelo `CMAKE_PREFIX_PATH`. Ele deve apontar para o prefixo de desenvolvimento ou instalacao do OBS que contenha os arquivos de pacote CMake do `libobs`. O modulo resultante e instalado no diretorio de plugins do OBS com o nome `filter-hotkeys-for-obs.dll`.

### Compilacao no macOS

Em um sistema macOS com Xcode e as dependencias de desenvolvimento do OBS instaladas, execute:

```bash
cmake --preset macos
cmake --build --preset macos --config RelWithDebInfo
cmake --install build_macos --config RelWithDebInfo
```

A assinatura de codigo pode ser necessaria ao instalar dentro de um bundle assinado do OBS.

### Estrutura para instalacao manual

Feche o OBS antes de copiar os arquivos. Use a estrutura correspondente a plataforma:

| Plataforma | Modulo | Dados de idioma |
|---|---|---|
| Windows x64 | `obs-plugins\\64bit\\filter-hotkeys-for-obs.dll` | `data\\obs-plugins\\filter-hotkeys-for-obs\\locale` |
| Linux x86_64 | `lib/x86_64-linux-gnu/obs-plugins/filter-hotkeys-for-obs.so` | `share/obs/obs-plugins/filter-hotkeys-for-obs/locale` |
| macOS | `filter-hotkeys-for-obs.plugin` | Dentro do bundle do plugin |

Reinicie o OBS depois da instalacao. O filtro fica disponivel ao adicionar um filtro a uma fonte com o nome **Filter Hotkeys - Controlador de filtros** ou sua traducao correspondente.

### Configuracao

1. Adicione a uma fonte o filtro que deseja controlar.
2. Adicione **Filter Hotkeys - Controlador de filtros** a mesma fonte.
3. Abra as propriedades do controlador.
4. Selecione o filtro-alvo na lista.
5. Escolha uma acao: **Alternar estado**, **Ativar filtro** ou **Desativar filtro**.
6. Digite uma tecla como `F9` ou `Ctrl+F9`.
7. Clique em **Salvar bind**.
8. Clique em **Testar acao agora** para confirmar o filtro-alvo e a acao escolhida.
9. Clique em **OK** e pressione a mesma tecla enquanto a janela principal do OBS estiver em foco.

E possivel adicionar varias instancias do controlador a mesma fonte. Use um filtro-alvo e uma tecla diferentes em cada instancia quando precisar de controles independentes.

### Formato aceito para teclas

O parser aceita o nome da tecla com modificadores opcionais separados por `+`:

```text
F1 ate F24
A ate Z
0 ate 9
Space
Enter
Escape
Ctrl+F9
Alt+F10
Ctrl+Shift+F11
```

O parser tambem aceita nomes internos do OBS, como `OBS_KEY_F9`, quando necessario. Os aliases de modificadores incluem `Ctrl`/`Control`, `Alt`, `Shift` e `Win`/`Meta`/`Cmd`.

### Solucao de problemas

Se o controlador nao aparecer, feche o OBS e confirme que a arquitetura do modulo corresponde a arquitetura do OBS. O OBS Studio 64-bit exige um plugin 64-bit.

Se a lista de filtros-alvo estiver vazia, adicione primeiro o filtro-alvo, salve a fonte, feche a janela de propriedades e abra novamente as propriedades do controlador.

Se **Testar acao agora** alterar o filtro, mas a tecla nao funcionar, confira o log do OBS. Uma configuracao bem-sucedida deve conter mensagens semelhantes a:

```text
Filter Hotkeys loaded successfully
Filter Hotkeys: binding aplicada 'F9' -> OBS_KEY_F9
Filter Hotkeys: callback disparado
```

O controlador usa o mecanismo de registro de hotkey frontend porque os filtros sao objetos privados de fonte no OBS. Portanto, a tecla nao deve ser procurada como uma hotkey de fonte comum ligada diretamente ao objeto filtro.

### Licenca e contato

Este projeto e distribuido sob a GNU General Public License versao 2 ou posterior. Para duvidas, relatos de problemas ou contribuicoes, entre em contato com Fabio Ventura pelo email [streamprogamers@proton.me](mailto:streamprogamers@proton.me).

---

## Author / Autor

**Fabio Ventura** — [streamprogamers@proton.me](mailto:streamprogamers@proton.me)
