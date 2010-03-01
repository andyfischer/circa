package
{
	import flash.display.Sprite;
	import cmodule.circa.CLibInit;
	
	public class Main extends Sprite
	{
		public function Main()
		{
			var loader:CLibInit = new CLibInit;
			var lib:Object = loader.init();
			trace(lib.echo("foo"));
		}
	}
}
