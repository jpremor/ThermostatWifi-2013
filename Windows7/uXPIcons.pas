//--------------------------------------------------------------------------------
// NOTE: You must include the XPMan unit for this fix to work in your application
//--------------------------------------------------------------------------------
unit uXPIcons;

interface

uses
  Windows, SysUtils, Classes, Graphics, Controls, Consts;

procedure AddIconFileToImageList(const FileName: string; IconIndex: Integer; const ImageList: TImageList);
function AddIconResourceToImageList(const ResourceName: string; const ImageList: TImageList): integer;
procedure ConvertTo32BitImageList(const ImageList: TImageList);
function GetFileIcon(const FileName: string; IconIndex: Integer): THandle;

implementation

uses
  ShellAPI, CommCtrl;

type
  PHICON = ^HICON;

function ExtractIconEx(lpszFile: PChar; nIconIndex: Integer;  phIconLarge, phIconSmall: PHICON; nIcons: UINT): UINT; stdcall; external shell32;

procedure ConvertTo32BitImageList(const ImageList: TImageList);
const
  Mask: array[Boolean] of Longint = (0, ILC_MASK);
var
  TempList: TImageList;
begin
  if Assigned(ImageList) then
  begin
    TempList := TImageList.Create(nil);
    try
      TempList.Assign(ImageList);
      with ImageList do
      begin
        Handle := ImageList_Create(Width, Height, ILC_COLOR32 or Mask[Masked], 0, AllocBy);

        if not HandleAllocated then
          raise EInvalidOperation.Create(SInvalidImageList);
      end;

      ImageList.AddImages(TempList);
    finally
      FreeAndNil(TempList);
    end;
  end;
end;

function GetFileIcon(const FileName: string; IconIndex: Integer): THandle;
var
  IconHandle: HICON;
  IconCount: UINT;
begin
  IconHandle := 0;
  IconCount := ExtractIconEx(PChar(FileName), IconIndex, @IconHandle, nil, 1);
  if (IconCount > 0) and (IconHandle > 0) then
    Result := IconHandle
  else
    Result := 0;
end;

procedure AddIconFileToImageList(const FileName: string; IconIndex: Integer; const ImageList: TImageList);
var
  TempIcon: TIcon;
begin
  TempIcon := TIcon.Create;
  try
    TempIcon.Width  := ImageList.Width;
    TempIcon.Height := ImageList.Height;
    TempIcon.Handle := GetFileIcon(FileName, IconIndex);
    if (TempIcon.Handle > 0) then
    begin
      ImageList.AddIcon(TempIcon);
      DestroyIcon(TempIcon.Handle);
    end;
  finally
    FreeAndNil(TempIcon);
  end;
end;

function AddIconResourceToImageList(const ResourceName: string; const ImageList: TImageList): integer;
var
  TempIcon: TIcon;
begin
  Result := -1;

  TempIcon := TIcon.Create;
  try
    TempIcon.Width  := ImageList.Width;
    TempIcon.Height := ImageList.Height;
    TempIcon.Handle := LoadIcon(HInstance, PChar(ResourceName));
    if (TempIcon.Handle > 0) then
    begin
      Result := ImageList.AddIcon(TempIcon);
      DestroyIcon(TempIcon.Handle);
    end;
  finally
    FreeAndNil(TempIcon);
  end;
end;

end.

