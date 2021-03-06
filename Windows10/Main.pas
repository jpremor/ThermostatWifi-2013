unit Main;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ScktComp, ExtCtrls, Gauges, ImgList, ComCtrls, ToolWin,
  Buttons, IniFiles, Grids, DBGrids, DB, DBClient, GIFImg, IdBaseComponent, IdComponent,
  IdTCPConnection, IdTCPClient, jpeg, NiceChart,
  Registry;

const

  DEFAULT_PORT            = 2001;

  HISTORY_FILE_NAME       = 'History.csv';

  AQUIS_PERIOD            = 60000;
  CLIENT_REQUEST_PERIOD   = 2000;
  
  LABEL_CONNECTED         = 'Connect';
  LABEL_DISCONNECTED      = 'Disconnect';

  MAX_CSV_COL 		        = 40;

  BARRA_CONEXAO       = 0;
  BARRA_COMUNIC       = 1;
  BARRA_RELOGIO       = 2;


type


  // tipo para ser usado
  TTCPCommands = record
    WaitingAnswer   : bool;
    AskTemp1        : bool;
    AskTemp2        : bool;
    AskRelayCool    : bool;
    AskRelayHeat    : bool;
    AskRelayFan     : bool;
    AskHeatSetpoint : bool;
    AskCoolSetpoint : bool;
    AskTempControlState : bool;
    AskTempControlMeasure : bool;
    AskStatusFull   : bool;
    AskStatusShort  : bool;
    SetHeatRefInc   : bool;
    SetHeatRefDec   : bool;
    SetNewHeat      : bool;
    SetCoolRefInc   : bool;
    SetCoolRefDec   : bool;
    SetNewCool      : bool;
    SetTempCtrl     : bool;
    SetForceFan     : bool;
  end;

  TConnectingStatus = record
    Tries           : integer;
    Trying          : bool;
    TryingIP        : string;
    LastWasSuccess  : bool; 
    LastSuccessFail : bool;
    SuccessIP       : string;
  end;

  TVars = record
    Temp1           : real;
    Temp2           : real;
    relaycool       : bool;
    relayheat       : bool;
    relayfan        : bool;
    heatsetpoint    : real;
    coolsetpoint    : real;
    controlstate    : integer;
    controlmeas     : real;
  end;

  TFormMain = class(TForm)
    ClientSocket: TClientSocket;
    TimerClientRequest: TTimer;
    StatusBar: TStatusBar;
    PageControl: TPageControl;
    ToolBarPrincipal: TToolBar;
    LabelIP: TLabel;
    EditIp: TEdit;
    TimerSearchIPs: TTimer;
    Label3: TLabel;
    EditIpUpTo: TEdit;
    TabSheetEthernet: TTabSheet;
    PageControlConfigPrograma: TPageControl;
    CheckBoxConectaStartup: TCheckBox;
    SpeedButtonConecta: TSpeedButton;
    SpeedButtonSair: TSpeedButton;
    Tab_Icones: TTabSheet;
    Tab_Graficos: TTabSheet;
    Tab_ConfigPrograma: TTabSheet;
    IconeMonitorGraficos: TImage;
    Label62: TLabel;
    IconeConfiguraPrograma: TImage;
    Image3: TImage;
    Label148: TLabel;
    Label150: TLabel;
    Image2: TImage;
    ImageLed: TImage;
    EditPort: TEdit;
    Label5: TLabel;
    LabelTemp1: TLabel;
    LabelTemp2: TLabel;
    LabelCoolingStatus: TLabel;
    Label1: TLabel;
    Label2: TLabel;
    Label4: TLabel;
    Label6: TLabel;
    LabelMeasure: TLabel;
    Label10: TLabel;
    LabelState: TLabel;
    Image1: TImage;
    Label7: TLabel;
    ImagePlusHeat: TImage;
    ImagePlusCool: TImage;
    ImageMinusHeat: TImage;
    ImageMinusCool: TImage;
    EditNewHeat: TEdit;
    EditNewCool: TEdit;
    Label8: TLabel;
    Label9: TLabel;
    CheckBoxTempCtrl: TCheckBox;
    LabelEnabled: TLabel;
    CheckBoxForceFan: TCheckBox;
    LabelFanForced: TLabel;
    MemoDebug: TMemo;
    Edit1: TEdit;
    MemoFile: TMemo;
    function FindStringData(Str : string; AfterStr: string; lengthStr : integer) : string;
    procedure TimerClientRequestTimer(Sender: TObject);
    procedure ClientSocketRead(Sender: TObject; Socket: TCustomWinSocket);
    procedure ClientSocketError(Sender: TObject; Socket: TCustomWinSocket;
      ErrorEvent: TErrorEvent; var ErrorCode: Integer);
    procedure FormCreate(Sender: TObject);
    procedure ClientSocketConnecting(Sender: TObject; Socket: TCustomWinSocket);
    procedure ClientSocketConnect(Sender: TObject; Socket: TCustomWinSocket);
    procedure ClientSocketDisconnect(Sender: TObject; Socket: TCustomWinSocket);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure Edit_Msc1Tq6Change(Sender: TObject);
    procedure TimerSearchIPsTimer(Sender: TObject);
    procedure SpeedButtonConectaClick(Sender: TObject);
    procedure EditSonum(Sender: TObject; var Key: Char);
    procedure EditMax255(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure EditMax65535(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure EditMaxAnoEntrada(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure EditMesEntrada(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure EditDiaEntrada(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure SpeedButtonSairClick(Sender: TObject);
    procedure SpeedButtonFundoClick(Sender: TObject);
    procedure SpeedButtonKeyboardClick(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure IconeRetornaDblClick(Sender: TObject);
    procedure IconeMonitorGraficosDblClick(Sender: TObject);
    procedure IconeConfiguraProgramaDblClick(Sender: TObject);
    procedure EditFloat(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure EditSonumFloat(Sender: TObject; var Key: Char);
    procedure ButtonHeatIncClick(Sender: TObject);
    procedure ButtonHeatDecClick(Sender: TObject);
    procedure ButtonCoolIncClick(Sender: TObject);
    procedure ButtonCoolDecClick(Sender: TObject);
    procedure CheckBoxConectaStartupClick(Sender: TObject);
    procedure EditIpUpToExit(Sender: TObject);
    procedure EditIpExit(Sender: TObject);
    procedure EditPortExit(Sender: TObject);
    procedure EditNewHeatExit(Sender: TObject);
    procedure EditNewCoolExit(Sender: TObject);
    procedure EditNewHeatEnter(Sender: TObject);
    procedure EditNewCoolEnter(Sender: TObject);
    procedure CheckBoxTempCtrlClick(Sender: TObject);
    procedure CheckBoxForceFanClick(Sender: TObject);
    procedure FormClick(Sender: TObject);
  private
    { Private declarations }
    procedure ButtonConfigSave;
    procedure ButtonLoadConfig;
    procedure EditsState(newset : boolean);
    procedure SetButtonDataRequest;
    procedure SplitStr(const Delimiter: Char;  Input: string; const Strings: TStrings) ;
    function ReturnIPToSearch(firstIP : string) : string;
  public
    { Public declarations }
    function HoraStrToMinutos(Value : string) : integer;
    procedure UpdateGraph;
    procedure UpdateSpreadsheet;
    procedure GraphMouseUp(Sender: TObject;
      Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
end;

var
  FormMain                : TFormMain;
  TCPCommands             : TTCPCommands;
  Vars                    : TVars;
  GraphTempCtrl           : TNiceChart;
    SerieTempMeas         : TNiceSeries;
    SerieRefCool          : TNiceSeries;
    SerieRefHeat          : TNiceSeries;
    SerieFanStat          : TNiceSeries;
    SerieTCtrlEn          : TNiceSeries;

  tempEditNewHeat, tempEditNewCool  : string;
  EditingNewHeat, EditingNewCool : boolean;
  CheckboxChangedByProcedure : boolean;

  ConnectingStatus  : TConnectingStatus;
  GraphUpdateIdx       : integer;

implementation

{$R *.dfm}

uses uXPIcons;













// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// PROCEDIMENTOS DE CARREGAMENTO E SALVAMENTO DE CONFIGURACAO DA GRANJA

// botao que joga para as caixas da data de entrada de frango a data atual...
//procedure TFormPrincipal.ButtonCarregaDataAtualClick(Sender: TObject);
//var
//  Ano, Mes, Dia : word;
//begin

  //DecodeDate(Date-1, Ano, Mes, Dia);

  //EditDiaReset.Text := inttostr(Dia);
  //EditMesReset.Text := inttostr(Mes);
  //EditAnoReset.Text := inttostr(Ano);
//end;


















// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// PROCEDIMENTOS DE USO DE CHECKBOXES


// click pARA HABILITAR uso de ethernet
function UseComma(Str : string) : string;
var
  i : integer;
  tempstr : string;
begin
  tempstr := '';
  for i := 1 to length(str) do begin
    if str[i]=',' then tempstr := tempstr + '.' else tempstr := tempstr + str[i];
  end;
  result := tempstr;
end;

procedure TFormMain.ButtonCoolDecClick(Sender: TObject);
begin
  if TImage(Sender).Enabled then begin
    TCPCommands.SetCoolRefDec := true;
    SetButtonDataRequest;
  end;
end;

procedure TFormMain.ButtonCoolIncClick(Sender: TObject);
begin
  if TImage(Sender).Enabled then begin
    TCPCommands.SetCoolRefInc := true;
    SetButtonDataRequest;
  end;
end;

procedure TFormMain.ButtonHeatDecClick(Sender: TObject);
begin
  if TImage(Sender).Enabled then begin
    TCPCommands.SetHeatRefDec := true;
    SetButtonDataRequest;
  end;
end;

procedure TFormMain.SetButtonDataRequest;
begin
  TimerClientRequest.Interval := CLIENT_REQUEST_PERIOD;
  TimerClientRequest.Enabled := true;
  MemoDebug.Lines.Add('TCP Request timer set to ENABLED');
  
  TimerSearchIPs.Interval := 10;
end;

procedure TFormMain.ButtonHeatIncClick(Sender: TObject);
begin
  if TImage(Sender).Enabled then begin
    TCPCommands.SetHeatRefInc := true;
    SetButtonDataRequest;
  end;
end;

procedure TFormMain.EditsState(newset : boolean);
begin
  if newset then begin
    EditNewHeat.Enabled := true;
    EditNewCool.Enabled := true;
    ImagePlusCool.Enabled := true;
    ImagePlusHeat.Enabled := true;
    ImageMinusCool.Enabled := true;
    ImageMinusHeat.Enabled := true;
    CheckBoxForceFan.Enabled := true;
    CheckBoxTempCtrl.Enabled := true;
  end
  else begin
    EditNewHeat.Enabled := false;
    EditNewCool.Enabled := false;
    ImagePlusCool.Enabled := false;
    ImagePlusHeat.Enabled := false;
    ImageMinusCool.Enabled := false;
    ImageMinusHeat.Enabled := false;
    CheckBoxForceFan.Enabled := false;
    CheckBoxTempCtrl.Enabled := false;
  end;
end;


function TFormMain.FindStringData(Str : string; AfterStr: string; lengthStr : integer) : string;
var
  j, i : integer;
  beginindex : integer;
  found : bool;

  output : string;
begin
  beginindex := 0;

  for i := 1 to length(Str) do begin
    //found first char...
    if Str[i] = AfterStr[1] then begin
      found := true;    // consider that it found

      for j := 2 to length(AfterStr) do begin

        if Str[j+i-1] <> AfterStr[j] then begin
          found := false;
          break;
        end;

      end;

      // got here with found, then set the begin index
      if found then begin
        beginindex := i+j-1;
        break;
      end;

    end;

  end;


  if beginindex=0 then begin
    output := '0';
  end
  else begin
    output := '';
    for i := beginindex to beginindex + lengthStr - 1 do begin
      if Str[i] <> '.' then output := output + Str[i]
      else output := output + ',';
    end;

  end;

  result := output;

end;



































// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// RECEBENDO DADOS NA COMUNICACAO
procedure TFormMain.ClientSocketRead(Sender: TObject; Socket: TCustomWinSocket);
var
  input : string;
  tempF : real;
  tempC : real;
  strtemp : string;
  state : integer;
  ReceivedCorrectAnsStatus : bool;
  ReceivedCorrectAnsWriting : bool;
begin

  // le buffer de entrada da eth
  input := Socket.ReceiveText;
  ReceivedCorrectAnsStatus := false; // if not waiting command
  ReceivedCorrectAnsWriting := false;
  // consider that no checkbox request was made by update
  CheckboxChangedByProcedure := false;

  // if there is a request...
  if TCPCommands.WaitingAnswer then begin

    MemoDebug.Lines.Add('TCP Client READING waiting answer');

    // check if the request is analog input 1
    if TCPCommands.AskTemp1 then begin
      ReceivedCorrectAnsStatus := true;
      try
        strtemp :=FindStringData(input, 'C : ', 5);
        tempC := StrToFloat(strtemp);
      except
        on Exception : EConvertError do tempC := 0;
      end;
      tempF := (tempC * 9 / 5) + 32;
      LabelTemp1.Caption := FloatToStrF(tempF,ffFixed,3,1) + ' F';
    end




    else if TCPCommands.AskTemp2 then begin
      ReceivedCorrectAnsStatus := true;
      try
        strtemp :=FindStringData(input, 'C : ', 5);
        tempC := StrToFloat(strtemp);
      except
        on Exception : EConvertError do tempC := 0;
      end;
      tempF := (tempC * 9 / 5) + 32;
      LabelTemp2.Caption := FloatToStrF(tempF,ffFixed,3,1) + ' F';
    end




    else if TCPCommands.AskRelayHeat then begin
      ReceivedCorrectAnsStatus := true;
      strtemp :=FindStringData(input, 'is ', 4);
      if (strtemp[1] = 'o') and (strtemp[2] = 'p') and (strtemp[3] = 'e') then begin
        LabelCoolingStatus.Caption := '';
      end
      else if (strtemp[1] = 'c') and (strtemp[2] = 'l') and (strtemp[3] = 'o') then begin
        LabelCoolingStatus.Caption := 'Heating';
      end;
    end
    else if TCPCommands.AskRelayCool then begin
      ReceivedCorrectAnsStatus := true;
      strtemp :=FindStringData(input, 'is ', 4);
      if (strtemp[1] = 'o') and (strtemp[2] = 'p') and (strtemp[3] = 'e') then begin
        LabelCoolingStatus.Caption := '';
      end
      else if (strtemp[1] = 'c') and (strtemp[2] = 'l') and (strtemp[3] = 'o') then begin
        LabelCoolingStatus.Caption := 'Cooling';
      end;
    end

    else if TCPCommands.AskRelayFan then begin
      ReceivedCorrectAnsStatus := true;
      Label7.Caption := input;
    end


    else if TCPCommands.AskHeatSetpoint then begin
      ReceivedCorrectAnsStatus := true;
      try
        strtemp :=FindStringData(input, 'heat = ', 5);
        tempC := StrToFloat(strtemp);
      except
        on Exception : EConvertError do tempC := 0;
      end;
      tempF := (tempC * 9 / 5) + 32;
      if EditingNewHeat = false then EditNewHeat.Text := FloatToStrF(tempF,ffFixed,3,1);
    end


    else if TCPCommands.AskCoolSetpoint then begin
      ReceivedCorrectAnsStatus := true;
      try
        strtemp :=FindStringData(input, 'cool = ', 5);
        tempC := StrToFloat(strtemp);
      except
        on Exception : EConvertError do tempC := 0;
      end;
      tempF := (tempC * 9 / 5) + 32;
      if EditingNewCool = false then EditNewCool.Text := FloatToStrF(tempF,ffFixed,3,1);
    end




    else if TCPCommands.AskTempControlState then begin
      ReceivedCorrectAnsStatus := true;
      try
        strtemp :=FindStringData(input, 'ate: ', 1);
        state := StrToInt(strtemp[1]);
      except
        on Exception : EConvertError do state := 2;
      end;
      if state = 1 then LabelState.Caption := 'Starting up...'
      else if state = 2 then LabelState.Caption := 'Satisfied'
      else if state = 3 then LabelState.Caption := 'Cooling'
      else if state = 4 then LabelState.Caption := 'Heating';
    end


    else if TCPCommands.AskTempControlMeasure then begin
      ReceivedCorrectAnsStatus := true;
      try
        strtemp :=FindStringData(input, 'sured: ', 5);
        tempC := StrToFloat(strtemp);
      except
        on Exception : EConvertError do tempC := 0;
      end;
      tempF := (tempC * 9 / 5) + 32;
      LabelMeasure.Caption := FloatToStrF(tempF,ffFixed,3,1) + ' F';
    end
















    
    // full answer variables
    else if TCPCommands.AskStatusFull then begin
      ReceivedCorrectAnsStatus := true;
      //received error
      if (input[1] = 'e') and (input[2] = ':') then begin
        ReceivedCorrectAnsStatus := false; // found error on the answer
        MemoDebug.Lines.Add('TCP Client READING error');
      end
      else begin
        StatusBar.Panels.Items[BARRA_COMUNIC].Text := 'Data in.';
        FormMain.EditsState(true);
        // temp state
        try
          strtemp :=FindStringData(input, 'tate:', 1);
          state := StrToInt(strtemp[1]);
        except
          on Exception : EConvertError do state := 2;
        end;
        if state = 1 then LabelState.Caption := 'Starting up...'
        else if state = 2 then LabelState.Caption := 'Satisfied'
        else if state = 3 then LabelState.Caption := 'Cooling'
        else if state = 4 then LabelState.Caption := 'Heating'
        else if state = 5 then LabelState.Caption := 'Disabled';
        Vars.controlstate := state;

        // temp measured
        try
          strtemp :=FindStringData(input, 'sured:', 5);
          tempC := StrToFloat(strtemp);
        except
          on Exception : EConvertError do tempC := 0;
        end;
        tempF := (tempC * 9 / 5) + 32;
        LabelMeasure.Caption := FloatToStrF(tempF,ffFixed,3,1) + ' F';
        Vars.controlmeas := tempF;

        // reference cool
        try
          strtemp :=FindStringData(input, 'refcool:', 5);
          tempC := StrToFloat(strtemp);
        except
          on Exception : EConvertError do tempC := 0;
        end;
        tempF := (tempC * 9 / 5) + 32;
        if EditingNewCool = false then EditNewCool.Text := FloatToStrF(tempF,ffFixed,3,1);
        Vars.coolsetpoint := tempF;

        // reference heat
        try
          strtemp :=FindStringData(input, 'refheat:', 5);
          tempC := StrToFloat(strtemp);
        except
          on Exception : EConvertError do tempC := 0;
        end;
        tempF := (tempC * 9 / 5) + 32;
        if EditingNewHeat = false then EditNewHeat.Text := FloatToStrF(tempF,ffFixed,3,1);
        Vars.heatsetpoint := tempF;

        // sens 1
        try
          strtemp :=FindStringData(input, 'sens1:', 5);
          tempC := StrToFloat(strtemp);
        except
          on Exception : EConvertError do tempC := 0;
        end;
        tempF := (tempC * 9 / 5) + 32;
        LabelTemp1.Caption := FloatToStrF(tempF,ffFixed,3,1) + ' F';
        Vars.Temp1 := tempF;

        // sens 2
        try
          strtemp :=FindStringData(input, 'sens2:', 5);
          tempC := StrToFloat(strtemp);
        except
          on Exception : EConvertError do tempC := 0;
        end;
        tempF := (tempC * 9 / 5) + 32;
        LabelTemp2.Caption := FloatToStrF(tempF,ffFixed,3,1) + ' F';
        Vars.Temp2 := tempF;

        // relay cool
        strtemp :=FindStringData(input, 'aycool:', 1);
        if strtemp[1] = '0' then begin
          LabelCoolingStatus.Caption := '';
          Vars.relaycool := false;
        end
        else if strtemp[1] = '1' then begin
          LabelCoolingStatus.Caption := 'Heating';
          Vars.relaycool := true;
        end;

        // relay heat
        strtemp :=FindStringData(input, 'ayheat:', 1);
        if strtemp[1] = '0' then begin
          LabelCoolingStatus.Caption := '';
          Vars.relayheat := false;
        end
        else if strtemp[1] = '1' then begin
          LabelCoolingStatus.Caption := 'Cooling';
          Vars.relayheat := true;
        end;

        // relay fan
        strtemp :=FindStringData(input, 'relay3:', 1);
        if strtemp[1] = '0' then begin
          LabelFanForced.Caption := 'Off';
          if CheckBoxForceFan.Checked then begin
            CheckBoxForceFan.Checked := false;
            CheckboxChangedByProcedure := true;
          end;
          Vars.relayfan := false;
        end
        else if strtemp[1] = '1' then begin
          LabelFanForced.Caption := 'On';
          if CheckBoxForceFan.Checked = false then begin          
            CheckBoxForceFan.Checked := true;
            CheckboxChangedByProcedure := true;
          end;
          Vars.relayfan := true;
        end;

        // temp control state
        strtemp :=FindStringData(input, 'tcenable:', 1);
        if (strtemp[1] = '1') then begin
          if CheckBoxTempCtrl.Checked = false then begin          
            CheckBoxTempCtrl.Checked := true;
            CheckboxChangedByProcedure := true;
          end;
          LabelEnabled.Caption := 'On';
        end
        else if (strtemp[1] = '0') then begin
          if CheckBoxTempCtrl.Checked then begin          
            CheckBoxTempCtrl.Checked := false;
            CheckboxChangedByProcedure := true;
          end;
          LabelEnabled.Caption := 'Off';
        end;

        // add new info to graphics
        UpdateSpreadsheet;
        UpdateGraph;


      end; // if not an error

    end 


    // writing answer
    else if TCPCommands.SetNewHeat then begin
      ReceivedCorrectAnsWriting := true;
      TCPCommands.SetNewHeat := false;
    end
    else if TCPCommands.SetHeatRefInc then begin
      ReceivedCorrectAnsWriting := true;
      TCPCommands.SetHeatRefInc := false;
    end
    else if TCPCommands.SetHeatRefDec then begin
      ReceivedCorrectAnsWriting := true;
      TCPCommands.SetHeatRefDec := false;
    end
    else if TCPCommands.SetCoolRefInc then begin
      ReceivedCorrectAnsWriting := true;
      TCPCommands.SetCoolRefInc := false;
    end
    else if TCPCommands.SetCoolRefDec then begin
      ReceivedCorrectAnsWriting := true;
      TCPCommands.SetCoolRefDec := false;
    end
    else if TCPCommands.SetNewCool then begin
      ReceivedCorrectAnsWriting := true;
      TCPCommands.SetNewCool := false;
    end
    else if TCPCommands.SetTempCtrl then begin
      ReceivedCorrectAnsWriting := true;
      TCPCommands.SetTempCtrl := false;
    end
    else if TCPCommands.SetForceFan then begin
      ReceivedCorrectAnsWriting := true;
      TCPCommands.SetForceFan := false;
    end
    
    // no active request found
    else begin
      ReceivedCorrectAnsStatus := false;
      ReceivedCorrectAnsWriting := false;
      MemoDebug.Lines.Add('TCP Client READING answer not expected');
    end;

  end
  else begin
    MemoDebug.Lines.Add('TCP Client READING... no request');
  end;


  if (ReceivedCorrectAnsStatus) then begin  
    TCPCommands.WaitingAnswer := false;
    MemoDebug.Lines.Add('TCP Client READED correct STATUS answer');
    Socket.Close;
  end
  else if ReceivedCorrectAnsWriting then begin
    TCPCommands.WaitingAnswer := false;
    MemoDebug.Lines.Add('TCP Client READED correct WRITING answer');  
    TimerClientRequest.Interval := CLIENT_REQUEST_PERIOD; // keep requesting for full status
  end;
    

end;







// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// GERENCIAMENTO DOS PEDIDOS
// *****************************
// timer de pedido modbus
procedure TFormMain.TimerClientRequestTimer(Sender: TObject);
var
  tempfloat : real;
  tempstr : string;
begin
    

    // primeiro atualiza o periodo do timer com o valor da caixa de edicao...
    StatusBar.Panels.Items[BARRA_COMUNIC].Text := '';
    TCPCommands.AskStatusFull := false; // dont ask for full status if any other request is up...

    // se a porta estiver aberta, inicia a varia??o de pedidos
    if (ClientSocket.Active = true) then begin
    
      // considering that any command will be send
      TCPCommands.WaitingAnswer := true;
      TimerClientRequest.Interval := AQUIS_PERIOD;

      // --------------------
      // wrinting commands
      if TCPCommands.SetCoolRefInc then begin
        tempfloat := (Vars.coolsetpoint + 1 -32) * 5 / 9;
        tempstr := FloatToStr(tempfloat);
        tempstr := UseComma(tempstr);
        ClientSocket.Socket.SendText('cs02'+tempstr[1]+tempstr[2]+tempstr[3]+tempstr[4]);
        TCPCommands.WaitingAnswer := true;
        MemoDebug.Lines.Add('TCP Request with client active : set cool ref increment');
      end
      else if TCPCommands.SetCoolRefDec then begin
        tempfloat := (Vars.coolsetpoint - 1 -32) * 5 / 9;
        tempstr := FloatToStr(tempfloat);
        tempstr := UseComma(tempstr);
        ClientSocket.Socket.SendText('cs02'+tempstr[1]+tempstr[2]+tempstr[3]+tempstr[4]);
        TCPCommands.WaitingAnswer := true;
        MemoDebug.Lines.Add('TCP Request with client active : set cool ref decrement');
      end
      else if TCPCommands.SetHeatRefInc then begin
        tempfloat := (Vars.heatsetpoint + 1 -32) * 5 / 9;
        tempstr := FloatToStr(tempfloat);
        tempstr := UseComma(tempstr);
        ClientSocket.Socket.SendText('cs03'+tempstr[1]+tempstr[2]+tempstr[3]+tempstr[4]);
        TCPCommands.WaitingAnswer := true;
        MemoDebug.Lines.Add('TCP Request with client active : set heat ref increment');
      end
      else if TCPCommands.SetHeatRefDec then begin
        tempfloat := (Vars.heatsetpoint - 1 -32) * 5 / 9;
        tempstr := FloatToStr(tempfloat);
        tempstr := UseComma(tempstr);
        ClientSocket.Socket.SendText('cs03'+tempstr[1]+tempstr[2]+tempstr[3]+tempstr[4]);
        TCPCommands.WaitingAnswer := true;
        MemoDebug.Lines.Add('TCP Request with client active : set heat ref decrement');
      end
      else if TCPCommands.SetNewHeat then begin
        tempfloat := (strtoFloat(EditNewHeat.Text)-32) * 5 / 9;
        tempstr := FloatToStr(tempfloat);
        tempstr := UseComma(tempstr);
        ClientSocket.Socket.SendText('cs03'+tempstr[1]+tempstr[2]+tempstr[3]+tempstr[4]);
        TCPCommands.WaitingAnswer := true;
        MemoDebug.Lines.Add('TCP Request with client active : set new heat ref');
      end
      else if TCPCommands.SetNewCool then begin
        tempfloat := (strtoFloat(EditNewCool.text)-32) * 5 / 9;
        tempstr := FloatToStr(tempfloat);
        tempstr := UseComma(tempstr);
        ClientSocket.Socket.SendText('cs02'+tempstr[1]+tempstr[2]+tempstr[3]+tempstr[4]);
        TCPCommands.WaitingAnswer := true;
        MemoDebug.Lines.Add('TCP Request with client active : set new cool ref');
      end
      else if TCPCommands.SetTempCtrl then begin
        if CheckBoxTempCtrl.Checked then tempstr:='1'
        else tempstr:='0';
        ClientSocket.Socket.SendText('cs15'+tempstr);
        TCPCommands.WaitingAnswer := true;
        MemoDebug.Lines.Add('TCP Request with client active : set temp ctrl');
      end
      else if TCPCommands.SetForceFan then begin
        if CheckBoxForceFan.Checked then ClientSocket.Socket.SendText('is3c')
        else ClientSocket.Socket.SendText('is3o');
        TCPCommands.WaitingAnswer := true;
        MemoDebug.Lines.Add('TCP Request with client active : set force fan');
      end




      // --------------------
      // reading commands
      else if (TCPCommands.AskTemp1 = true) then begin
        ClientSocket.Socket.SendText('ar1');
        TCPCommands.WaitingAnswer := true;
      end

{      else if (TCPCommands.AskTemp2 = true) then begin
        ClientSocket.Socket.SendText('ar2');
        TCPCommands.WaitingAnswer := true;
      end
      else if (TCPCommands.AskRelayHeat = true) then begin
        ClientSocket.Socket.SendText('ir1');
        TCPCommands.WaitingAnswer := true;
      end
      else if (TCPCommands.AskRelayCool = true) then begin
        ClientSocket.Socket.SendText('ir2');
        TCPCommands.WaitingAnswer := true;
      end
      else if (TCPCommands.AskRelayFan = true) then begin
        ClientSocket.Socket.SendText('ir3');
        TCPCommands.WaitingAnswer := true;
      end
      else if (TCPCommands.AskHeatSetpoint = true) then begin
        ClientSocket.Socket.SendText('cr03');
        TCPCommands.WaitingAnswer := true;
      end
      else if (TCPCommands.AskCoolSetpoint = true) then begin
        ClientSocket.Socket.SendText('cr02');
        TCPCommands.WaitingAnswer := true;
      end
      else if (TCPCommands.AskTempControlState) then begin
        ClientSocket.Socket.SendText('vr3');
        TCPCommands.WaitingAnswer := true;
      end
      else if (TCPCommands.AskTempControlMeasure) then begin
        ClientSocket.Socket.SendText('vr4');
        TCPCommands.WaitingAnswer := true;
      end
}      // no command sent


      // if there is not other request, ask full status
      else begin
        TCPCommands.AskStatusFull := true;
        ClientSocket.Socket.SendText('sf');
        TCPCommands.WaitingAnswer := true;
        MemoDebug.Lines.Add('TCP Request with client active : read full status');
      end



    end
    else begin
      MemoDebug.Lines.Add('TCP Request with client INACTIVE');
    end;
end;

// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// FIM GERENCIAMENTO DOS PEDIDOS
// *******************************
// *******************************
// *******************************
// *******************************
// *******************************




















// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// GERENCIAMENTO DA INSTALACAO DO SOFTWARE
// *******************************
// *******************************
// *******************************
// *******************************
// *******************************


// Retorna a diferenca de dias entre duas datas
function DifDias(DataInst:TDateTime; DataAtual:TDateTime): integer;
Var
  Data: TDateTime;
  dia, mes, ano: Word;
begin
if DataAtual < DataInst then
   begin
      Result := -1;
//   Result := 'A data data atual n?o pode ser menor que a data inicial';
   end
else
   begin
   Data := DataAtual - DataInst;
   DecodeDate( Data, ano, mes, dia);
   Result := trunc(Data);
   end;
end;

// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// FIM GERENCIAMENTO DE INSTALACAO DO SOFTWARE
// *******************************
// *******************************
// *******************************
// *******************************
// *******************************


























// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// FIM GERENCIAMENTO DA CONFIG
// *******************************
// *******************************
// *******************************
// *******************************
// *******************************

// salva configuracoes no arquivo ini
procedure TFormMain.ButtonConfigSave;
var
  ArqIni : TIniFile;
  tempstr : string;
begin
  SetCurrentDir(Application.ExeName);
  ArqIni := TIniFile.Create('config.ini');

  // salva os parametros configurados
  // aqui so salva os parametros que precisam que o botao seja apertado
  // nos eventos de alternancia de estado salva-se todos os parametros q atualizam
  // com a alteracoa do estado
  ArqIni.WriteString('EthBridgeConf','EndIpRemoto_1_A',tempstr);
  if tempstr <> null then
    FormMain.EditIp.Text := tempstr;
  ArqIni.WriteString('EthBridgeConf','EndIpRemoto_2_D',FormMain.EditIpUpTo.Text);
  ArqIni.WriteString('EthBridgeConf','PortaRemoto',FormMain.EditPort.Text);
  ArqIni.WriteBool('EthBridgeConf','ConectaInicializacao', FormMain.CheckBoxConectaStartup.Checked);

  ArqIni.Free;
end;


// recarrega configuracoes do arquivo ini.
procedure TFormMain.ButtonLoadConfig;
var
  GeralIni : TIniFile;
begin
  SetCurrentDir(Application.ExeName);
  GeralIni := TIniFile.Create('./config.ini');

  //******************************************
  // le parametros de configuracao da ethernet
  EditIp.Text   := GeralIni.ReadString('EthBridgeConf','EndIpRemoto_1_A','0');
  EditIpUpTo.Text   := GeralIni.ReadString('EthBridgeConf','EndIpRemoto_2_D','0');
  EditPort.Text    := GeralIni.ReadString('EthBridgeConf','PortaRemoto',inttostr(DEFAULT_PORT));
  CheckBoxConectaStartup.Checked := GeralIni.ReadBool('EthBridgeConf','StartupConnection', true);

  GeralIni.free;

 end;

// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// FIM GERENCIAMENTO DA CONFIG
// *******************************
// *******************************
// *******************************
// *******************************
// *******************************



























// *****************************
// Fechando o programa
procedure TFormMain.FormClick(Sender: TObject);
begin
  edit1.SetFocus;
end;

procedure TFormMain.FormClose(Sender: TObject; var Action: TCloseAction);
begin
    ButtonConfigSave;
end;

// *****************************
// Abrindo o programa
procedure TFormMain.FormCreate(Sender: TObject);
begin
  // draw the graph
  GraphTempCtrl := TNiceChart.Create(Self);
  GraphTempCtrl.Parent := Tab_Graficos;
  GraphTempCtrl.Height := 265;
  GraphTempCtrl.Width := 560;
  GraphTempCtrl.Top := 43;
  GraphTempCtrl.Left := 3;
  GraphTempCtrl.ShowTitle := false;
  GraphTempCtrl.AxisXTitle := 'time';
  GraphTempCtrl.AxisXScale := 10;
  GraphTempCtrl.AxisYScale := 10;
  GraphTempCtrl.ShowXGrid := true;
  GraphTempCtrl.ShowYGrid := true;
  GraphTempCtrl.ShowLegend := true;
  GraphTempCtrl.BorderStyle := bsNone;
  GraphTempCtrl.AxisYScale := 1;
  GraphTempCtrl.AxisXScale := 1;
  GraphTempCtrl.BevelKind := bkNone;
  GraphTempCtrl.BevelInner := bvNone;
  GraphTempCtrl.BevelOuter := bvNone;

  //GraphTempCtrl.BeginUpdate;
  SerieTempMeas := GraphTempCtrl.AddSeries(skSmooth);
  SerieRefHeat := GraphTempCtrl.AddSeries(skLine);
  SerieRefCool := GraphTempCtrl.AddSeries(skLine);
  SerieFanStat := GraphTempCtrl.AddSeries(skLine);
  SerieTCtrlEn := GraphTempCtrl.AddSeries(skLine);
  //GraphTempCtrl.EndUpdate;

  // get history file...
  SetCurrentDir(Application.ExeName);
  MemoFile.Lines.LoadFromFile(HISTORY_FILE_NAME);
  GraphUpdateIdx := 0;
  UpdateGraph;

  // load configurations
  ButtonLoadConfig;
  // verifica se tem q conectar automatico na inicializacao
  if (CheckBoxConectaStartup.Checked) and (SpeedButtonConecta.Caption = LABEL_CONNECTED) then
    SpeedButtonConecta.Click;

  TCPCommands.AskStatusFull := true;

  // inicializa variaveis
  ConnectingStatus.Tries := 0;

end;




                      








procedure TFormMain.GraphMouseUp(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
//  Grafico1Princ. .ScaleBy(4,4);
end;

procedure TFormMain.UpdateSpreadsheet;
begin
  MemoFile.Lines.Add(   DateToStr(Date)+';'+
                        TimeToStr(Time)+';'+
                        floatToStrF(Vars.controlmeas,ffFixed,3,1)+';'+
                        floatToStrF(Vars.coolsetpoint,ffFixed,3,1)+';'+
                        floatToStrF(Vars.heatsetpoint,ffFixed,3,1)+';'+
                        intToStr(Vars.controlstate)+';'+
                        boolToStr(Vars.relayfan)+';'
                        );
  SetCurrentDir(Application.ExeName);
  MemoFile.Lines.SaveToFile(HISTORY_FILE_NAME);
end;


procedure TFormMain.UpdateGraph;
var
//  refcool : real;
//  refheat : real;
//  tempctrl : integer;
//  forcefan : bool;
  Xval : TDateTime;
  I: Integer;
  Str,DateStr,TimeStr : TStringList;
begin
  GraphTempCtrl.BeginUpdate;

  Str := TStringList.Create;
  DateStr := TStringList.Create;
  TimeStr := TStringList.Create;
  try
    // set graph caption
    if GraphUpdateIdx = 0 then begin
      SerieTempMeas.caption := 'Temperature';
      SerieRefHeat.caption := 'Ref Heat';
      SerieRefCool.caption := 'Ref Cool';
      SerieFanStat.caption := 'Fan State';
      SerieTCtrlEn.caption := 'Ctrl State';
      GraphUpdateIdx := 1;
    end;

    for I := GraphUpdateIdx to MemoFile.Lines.Count-1 do begin
      Str.Clear;
      SplitStr(';',MemoFile.Lines[I],Str);
      SplitStr('/',Str[0],DateStr);
      SplitStr(':',Str[1],TimeStr);
      Xval := EncodeDate(Word(StrToInt(DateStr[2])),Word(StrToInt(DateStr[1])),Word(StrToInt(DateStr[0])))+
              EncodeTime(Word(StrToInt(TimeStr[0])),Word(StrToInt(TimeStr[1])),Word(StrToInt(TimeStr[2])),Word(0));

      SerieTempMeas.AddXY(Xval,StrToFloat(Str[2]));
      SerieRefCool.AddXY(Xval,StrToFloat(Str[3]));
      SerieRefHeat.AddXY(Xval,StrToFloat(Str[4]));
      SerieTCtrlEn.AddXY(Xval,StrToInt(Str[5])*10);
      if Str[6] = '0' then SerieFanStat.AddXY(Xval,50)
      else SerieFanStat.AddXY(Xval,0);

      GraphUpdateIdx := GraphUpdateIdx + 1;
    end;

    GraphTempCtrl.Repaint;
    GraphTempCtrl.EndUpdate;
  finally
    Str.Free;
    DateStr.Free;
    TimeStr.Free;
  end;

end;



procedure TFormMain.CheckBoxConectaStartupClick(Sender: TObject);
begin
  ButtonConfigSave;
end;

procedure TFormMain.CheckBoxForceFanClick(Sender: TObject);
begin
    TCPCommands.SetForceFan := true;
    if CheckboxChangedByProcedure = false then SetButtonDataRequest;
    CheckboxChangedByProcedure := false;
end;

procedure TFormMain.CheckBoxTempCtrlClick(Sender: TObject);
begin
    TCPCommands.SetTempCtrl := true;
    if CheckboxChangedByProcedure = false then SetButtonDataRequest;
    CheckboxChangedByProcedure := false;
end;

procedure TFormMain.FormShow(Sender: TObject);
//var
//  i : integer;
begin
  Tab_Graficos;

  doublebuffered := true;

  PageControl.Top := 0;
  PageControl.ActivePageIndex := Tab_Icones.PageIndex;
end;

// ****************************
// Controla as trocas de paginas nas configuracoes
procedure TFormMain.SpeedButtonConectaClick(Sender: TObject);
begin
  // se for para conectar
  if SpeedButtonConecta.Caption = LABEL_CONNECTED then begin
      SpeedButtonConecta.Caption := LABEL_DISCONNECTED;

      TimerSearchIPs.Enabled := true;
      StatusBar.Panels.Items[BARRA_CONEXAO].Text := 'Buscando conex?o';
  end
  // ? para desconectar
  else begin
      SpeedButtonConecta.Caption := LABEL_CONNECTED;

      StatusBar.Panels.Items[BARRA_CONEXAO].Text := 'Desconectado pelo usuario';
      // se for pra desconectar ethernet
      TimerSearchIPs.Enabled := false;
      ClientSocket.Active := false;
  end;
end;


  










// *****************************
// minimiza aplicativo
procedure TFormMain.SpeedButtonFundoClick(Sender: TObject);
begin
  FormMain.WindowState := wsMinimized;
end;

// *****************************
// conectado sem problemas
procedure TFormMain.SpeedButtonKeyboardClick(Sender: TObject);
//const
//var
//  Hwnd : THandle;
begin
//     Hwnd:= FindWindow( nil, 'Teclado Virtual' );
//     if Hwnd <> 0 then begin
//        //if not IsWindowVisible(Handle) then
//            ShowWindow(Hwnd, SW_RESTORE);
//        SetForegroundWindow(Hwnd);
//     end;

// to show keyboard
  PostMessage(FindWindow(nil, 'Teclado Virtual'), WM_ACTIVATE, 1, 0);
  PostMessage(FindWindow(nil, 'Teclado Virtual'), WM_MDIMAXIMIZE, 1, 0);

end;


// *****************************
// botao sair
procedure TFormMain.SpeedButtonSairClick(Sender: TObject);
begin
  FormMain.Close;
end;

// *****************************
// conectado sem problemas
procedure TFormMain.ClientSocketConnect(Sender: TObject; Socket: TCustomWinSocket);
begin
  // avisa que conectou
  StatusBar.Panels.Items[BARRA_CONEXAO].Text := 'Connected';

  ImageLed.Picture.LoadFromFile(ExtractFilePath(Application.ExeName) + '\pic\LedOn.bmp');

  // desliga timer de tentativa de conectar
  TimerSearchIPs.Interval := AQUIS_PERIOD; // one minute interval
  TimerClientRequest.interval := CLIENT_REQUEST_PERIOD; // start the request right way
  ConnectingStatus.LastWasSuccess := true;
  ConnectingStatus.SuccessIP := ConnectingStatus.TryingIP;  // set the tryingIP as the last success

  TimerClientRequest.Enabled := true;
  MemoDebug.Lines.Add('TCP Request timer set to ENABLED');

  MemoDebug.Lines.Add('TCP Client connection success');

end;

// *****************************
// trata erros no tcp
procedure TFormMain.ClientSocketError(Sender: TObject; Socket: TCustomWinSocket;
  ErrorEvent: TErrorEvent; var ErrorCode: Integer);
begin
  MemoDebug.Lines.Add('TCP Client connection error');
  
  // fecha o socket
  ClientSocket.Active := false;
  Socket.Close;

  if ConnectingStatus.LastWasSuccess then begin
    ConnectingStatus.LastSuccessFail := true;
    ConnectingStatus.LastWasSuccess := false;
  end;

  // limpa o error code para nao indicar mensagem
  ErrorCode := 0;

  // coloca status na barra
  StatusBar.Panels.Items[BARRA_CONEXAO].Text := 'Falha na conex?o';

  // liga o timer de reconex?o
  TimerSearchIPs.Interval := 300;
  StatusBar.Panels.Items[BARRA_COMUNIC].Text := '';
end;

// *****************************
// gera mensagens de erro
procedure TFormMain.ClientSocketConnecting(Sender: TObject;
  Socket: TCustomWinSocket);
begin
  StatusBar.Panels.Items[0].Text := 'Conectando';
  MemoDebug.Lines.Add('TCP Client connecting');
end;

procedure TFormMain.ClientSocketDisconnect(Sender: TObject;
  Socket: TCustomWinSocket);
begin
  ImageLed.Picture.LoadFromFile(ExtractFilePath(Application.ExeName) + '\pic\LedOff.bmp');
  //FormMain.EditsState(false);
  //StatusBar.Panels.Items[0].Text := 'Desconectado';
  //TimerPedidoModbusTcp.Enabled := false;
  StatusBar.Panels.Items[BARRA_COMUNIC].Text := '';

  ConnectingStatus.Trying := false;
  TimerClientRequest.Enabled := false;
  MemoDebug.Lines.Add('TCP Request timer set to DISABLED');
  
  MemoDebug.Lines.Add('TCP Client disconnected');
end;


procedure TFormMain.SplitStr(const Delimiter: Char;  Input: string; const Strings: TStrings) ;
begin
   Assert(Assigned(Strings));
   Strings.Clear;
   Strings.Delimiter := Delimiter;
   Strings.DelimitedText := Input;
end;

function TFormMain.ReturnIPToSearch(firstIP : string) : string;
var
  IP : TStringList;
  index : integer;
begin
  IP := TStringList.Create;
  try
    SplitStr('.',firstIP,IP);
    if StrToInt(IP[3]) + ConnectingStatus.Tries >= strtoint(EditIpUpTo.Text) then begin
      index := 0;
      ConnectingStatus.Tries := 0;
    end
    else index := ConnectingStatus.Tries;
    result := IP[0] + '.' + IP[1] + '.' + IP[2] + '.' +
            inttostr( StrToInt(IP[3]) + index );
  finally
    IP.Free;
  end;
end;



// *****************************
// reconecta se detectou erro na conexao
procedure TFormMain.TimerSearchIPsTimer(Sender: TObject);
begin
  TimerSearchIPs.Interval := AQUIS_PERIOD;

  // se nao tem conexao nenhuma, tenta conectar em algum lugar
    if ClientSocket.Active = false then begin

    // *****************************
    // primeiro configura ip e porta
    ClientSocket.Port := StrToIntDef(EditPort.Text,DEFAULT_PORT);
    // se deu erro na conversao strtoint, reescreve valor correto dentro da caixa porta
    if EditPort.Text <> inttostr(ClientSocket.Port) then EditPort.Text := inttostr(DEFAULT_PORT);

    
    // last connection was sucessfull
    if ConnectingStatus.LastWasSuccess then begin
      ClientSocket.Host :=  ConnectingStatus.SuccessIP;
      ConnectingStatus.Tries := 0;
      MemoDebug.Lines.Add('Connection request: Using last success');
    end
    else begin
      // ip eh pego partindo do primeiro da faixa configurada
      ClientSocket.Host :=  ReturnIPToSearch(EditIp.Text);
      StatusBar.Panels.Items[BARRA_CONEXAO].Text := 'Searching module...';
      MemoDebug.Lines.Add('Connection request: Searching module');
    end;
    
    // ativa a conexao
    ConnectingStatus.Tries := ConnectingStatus.Tries + 1;
    ConnectingStatus.TryingIP := ClientSocket.Host;
    ConnectingStatus.Trying := true;
    ClientSocket.Active := true;

  
    // desliga o timer de tentativa de conectar
    //TimerSearchIPs.Enabled := false;

  end;

end;



















// *******************************
// *******************************
// *******************************
// *******************************
// *******************************
// GERENCIAMENTO DAS CAIXAS DE TEXTO

// *****************************
// verifica se digitou apenas numeros inteiros
procedure TFormMain.EditSonum(Sender: TObject; var Key: Char);
begin
  if ((ord(Key) < 48) or (ord(Key) > 57))
  and (Key <> #8) and (Key <> #13) and (Key <> #10)
  then Key := #0;
end;

// *****************************
// verifica se digitou apenas numeros float (com ,)
procedure TFormMain.EditSonumFloat(Sender: TObject;
  var Key: Char);
begin
  if ((ord(Key) < 48) or (ord(Key) > 57))
  and (Key <> #8) and (Key <> #13) and (Key <> #10)
  and (Key <> ',')
  then Key := #0;
end;

// *****************************
// verifica se o ano digitado na daixa de entrada do frango eh menor q a data atual
procedure TFormMain.EditMaxAnoEntrada(Sender: TObject; var Key: Word;
  Shift: TShiftState);
var
  DiaH,MesH,AnoH : WORD;
begin
  if length(TEdit(Sender).Text) > 1 then
    if StrToIntDef(TEdit(Sender).Text,99999) > 9999 then begin
      TEdit(Sender).Text := '9999';
      Key := ord(0);
    end;

  DecodeDate(Date-1, AnoH, MesH, DiaH);
end;

// *****************************
// verifica se o mes digitado na daixa de entrada do frango eh menor q a data atual
procedure TFormMain.EditMesEntrada(Sender: TObject; var Key: Word;
  Shift: TShiftState);
var
  DiaH,MesH,AnoH : WORD;
begin
  if length(TEdit(Sender).Text) > 1 then
    if StrToIntDef(TEdit(Sender).Text,13) > 12 then begin
      TEdit(Sender).Text := '12';
      Key := ord(0);
    end;

  DecodeDate(Date-1, AnoH, MesH, DiaH);

end;



procedure TFormMain.EditPortExit(Sender: TObject);
begin
  ButtonConfigSave;
end;



// ******************************
// verifica se o valor digitado ? tipo float
procedure TFormMain.EditFloat(Sender: TObject;
  var Key: Word; Shift: TShiftState);
begin
  if length(TEdit(Sender).Text) > 1 then
    if StrToFloatDef(TEdit(Sender).Text,10001020110) = 10001020110 then begin
      TEdit(Sender).Text := '0,0';
      Key := ord(0);
    end;
end;

procedure TFormMain.EditIpExit(Sender: TObject);
begin
  ButtonConfigSave;
end;

procedure TFormMain.EditIpUpToExit(Sender: TObject);
begin
  ButtonConfigSave;
end;

// *****************************
// verifica se o mes digitado na daixa de entrada do frango eh menor q a data atual
procedure TFormMain.EditNewCoolEnter(Sender: TObject);
begin
  tempEditNewCool := EditNewCool.Text;
  EditingNewCool := true;
end;

procedure TFormMain.EditNewCoolExit(Sender: TObject);
begin
  // check the interval of the new setting
  if TCheckBox(Sender).Enabled then begin

    if length(TEdit(Sender).Text) > 1 then begin
      if StrToIntDef(TEdit(Sender).Text,0) > 255 then begin
        TEdit(Sender).Text := '255';
      end
      else if StrToIntDef(TEdit(Sender).Text,0) < 0 then begin
        TEdit(Sender).Text := '0';
      end
      else begin
        // no limits reached
        if tempEditNewCool <> EditNewCool.Text then begin
          TCPCommands.SetNewCool := true;
          SetButtonDataRequest;
        end;
        EditingNewCool := false;
      end;
    end; // if there is enougl data inside the box
  end; // if checkbox is enabled
  edit1.SetFocus;
end;

procedure TFormMain.EditNewHeatEnter(Sender: TObject);
begin
  tempEditNewHeat := EditNewHeat.Text;
  EditingNewHeat := true;
end;

procedure TFormMain.EditNewHeatExit(Sender: TObject);
begin
  // check the interval of the new setting
  if (length(TEdit(Sender).Text) > 1) and (TCheckbox(Sender).Enabled) then begin
    if StrToIntDef(TEdit(Sender).Text,0) > 255 then begin
      TEdit(Sender).Text := '255';
    end
    else if StrToIntDef(TEdit(Sender).Text,0) < 0 then begin
      TEdit(Sender).Text := '0';
    end
    else begin
      // no limits reached
      if tempEditNewHeat <> EditNewHeat.Text then begin
        TCPCommands.SetNewHeat := true;
        SetButtonDataRequest;
      end;
      EditingNewHeat := false;
    end;
  end;
  edit1.SetFocus;
end;

procedure TFormMain.EditDiaEntrada(Sender: TObject; var Key: Word;
  Shift: TShiftState);
var
  DiaH,MesH,AnoH : WORD;
begin
  if length(TEdit(Sender).Text) > 1 then
    if StrToIntDef(TEdit(Sender).Text,32) > 31 then begin
      TEdit(Sender).Text := '31';
      Key := ord(0);
    end;

  DecodeDate(Date-1, AnoH, MesH, DiaH);

end;

// *****************************
// max 255
procedure TFormMain.EditMax255(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if length(TEdit(Sender).Text) > 1 then
    if StrToIntDef(TEdit(Sender).Text,0) > 255 then begin
      TEdit(Sender).Text := '255';
      Key := ord(0);
    end;
end;


// *****************************
// max 65535
procedure TFormMain.EditMax65535(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if length(TEdit(Sender).Text) > 1 then
    if StrToIntDef(TEdit(Sender).Text,0) > 65535 then begin
      TEdit(Sender).Text := '65535';
      Key := ord(0);
    end;
end;

// ***************************************
// verifica se a caixa ta vazia
procedure TFormMain.Edit_Msc1Tq6Change(Sender: TObject);
begin
  if TEdit(Sender).Text = '' then TEdit(Sender).Text := '0';
end;








// *******************
// verifica tecla apertada dentro das caixas de texto da iluminacao
// tecla apertada


// converte formato HH:MM de string em tempo em segundos
function TFormMain.HoraStrToMinutos(Value : string) : integer;
var
  i : integer;
  HHstr, MMstr : string;
  Encontrou : boolean;
begin
  // varre atras do : e separa as string HH e MM
  Encontrou := false;
  HHstr := '';
  MMstr := '';

  // varre atras das strings separadas
  for i := 1 to length(Value) do begin
    if (Value[i] <> ':') then begin
      if (Encontrou = false) then
        HHstr := HHstr + Value[i]
      else MMstr := MMstr + Value[i];
    end
    else begin
      Encontrou := true;
    end;
  end;

  // verifica se encontra string diferente de numero na HHstr e no MMstr
  //Encontrou := false;
  for i := 1 to length(HHstr) do
    if ((ord(HHstr[i]) < 48) or (ord(HHstr[i]) > 57)) then begin
      HHstr := '00';
      break;
    end;
  for i := 1 to length(MMstr) do
    if ((ord(MMstr[i]) < 48) or (ord(MMstr[i]) > 57)) then begin
      MMstr := '00';
      break;
    end;

  result := ( (StrToIntDef(HHstr,0) * 60) + StrToIntDef(MMstr,0));

end;















































// ********************************
// clique duplo nos icones de telas

procedure TFormMain.IconeRetornaDblClick(Sender: TObject);
begin
    PageControl.ActivePageIndex := Tab_Icones.PageIndex;
    Edit1.SetFocus;
end;

procedure TFormMain.IconeConfiguraProgramaDblClick(Sender: TObject);
begin
    PageControl.ActivePageIndex := Tab_ConfigPrograma.PageIndex;
end;

procedure TFormMain.IconeMonitorGraficosDblClick(Sender: TObject);
begin
    PageControl.ActivePageIndex := Tab_Graficos.PageIndex;
end;



end.
