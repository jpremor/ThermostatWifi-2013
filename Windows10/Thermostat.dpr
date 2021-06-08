program Thermostat;

uses
  Forms,
  Main in 'Main.pas' {FormMain};

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.ShowMainform := true;
  Application.Title := 'Thermostat';
  Application.CreateForm(TFormMain, FormMain);
  Application.Run;
end.
