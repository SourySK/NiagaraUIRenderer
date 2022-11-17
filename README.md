# Niagara UI Renderer | Free Plugin for Unreal Engine

Note: UE5 branch was merged into main, UE4 now has a separate branch

Niagara UI Plugin adds Niagara Particle System Widget that allows you to render Niagara particle systems directly into the UI. The plugin supports sprite and ribbon CPU particles. It was tested on Windows and Android, but it should work fine on other platforms too.

Demo Video:  
[![Niagara UI Renderer Demo Video](http://img.youtube.com/vi/iFa40Sf4QPA/0.jpg)](http://www.youtube.com/watch?v=iFa40Sf4QPA "Niagara UI Renderer Demo")



## How to install:

You have 4 options how to install plugin:
1. **Install the plugin from the Unreal Marketplace into Unreal Engine:**
The easiest way to get this plugin. Installs directly into the engine so you can use it across multiple projects.
* Get the plugin from the [Unreal Marketplace](https://www.unrealengine.com/marketplace/en-US/product/niagara-ui-renderer)
* Download the plugin and install it into your engine
* Enable Niagara UI Renderer in your project’s Plugins settings
* Enjoy!

2. **Install precompiled plugin into Unreal Engine:**  
Recommended if you want to use plugin across multiple projects.  
Installation Steps:  
* Download the precompiled plugin for your version of Unreal Engine  
* Extract the whole plugin folder into ..\..\YourEngineInstallationPath\Engine\Plugins\  
* Enable Niagara UI Renderer in your projects Plugins settings  
* Enjoy!

3. **Install precompiled plugin into your project only:**  
Recommended if you want to use plugin in this project only.  
Installation Steps:  
* Download the precompiled plugin for your version of Unreal Engine  
* Extract the whole plugin folder into ..\..\YourProjectPath\Plugins\  
* Enable Niagara UI Renderer in your projects Plugins settings  
* Enjoy!

4. **Compile your own version from GitHub:**  
Recommended if you're Unreal Engine God.  
Installation Steps:  
* Download the plugin source from the GitHub repository  
* Copy the plugin into your plugins folder  
* Regenerate Visual Studio project files  
* Recompile  
* Enable Niagara UI Renderer in your projects Plugins settings  
* Enjoy!



## How to use:  
1. Create your Niagara particle system

2. Because Unreal cannot render "normal" particle materials as UI elements we'll need to create a version of each particle material with "User Interface" material domain. We'll also need to swap out some nodes. Use Vertex Color node instead of Particle Color node. Usual particle parameters such as Particle Radius, Particle Random won't be available for UI particle materials. For sprites only you can access Dynamic Parameter R and G data from Texture Coordinate with coordinate Index 1.

3. Add Niagara System Widget into your widget (you can find it in the widget palette under Niagara category)

4. Select your Niagara System in the Details tab

5. You still won't be able to see your particles, because we need to apply the newly created UI material. There are two ways we can to that: You can either set the material directly into your Particle System renderer. This will result in particle rendering correctly in UI, but it will make it render incorrectly in the Niagara preview and in the world. To avoid that, you can use Material Remap List on the widget itself. Just add new element into it, select the original particle material on the left, and select the newly created UI material on the right. You can do this for as many materials as you want. Each material on the left will get automatically remapped to the one on the right when rendering UI. This way you can still see the correct preview of the particle system, use the particle in the world and use it as a UI particle system!

6. You can choose if you want the particle system to auto activate, or you can activate it with blueprints.

7. Your UI particle is set up and ready to go! You can animate it, activate it on button press or do whatever else you want!



## For more information and images please visit the [Unreal Forum post](https://forums.unrealengine.com/t/niagara-ui-renderer-free-plugin-for-ue4/211365)

## You may be interested in the [Documentation](https://sourysk.github.io/NiagaraUIRendererDocumentation/)

## Or download from the [Unreal Marketplace](https://www.unrealengine.com/marketplace/en-US/product/niagara-ui-renderer)
