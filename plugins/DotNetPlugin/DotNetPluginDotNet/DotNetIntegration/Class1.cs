using tdotnetbridge.ClientLibrary;

namespace DotNetIntegration;

[QObject]
public class DotNetProject
{
    [ExportToQt]
    public DotNetProject()
    {
        
    }
    
    [ExportToQt]
    public string HelloWorld()
    {
        return "Hello World!";
    }
}
