// [_10/_11.js] 音频解码调度: _979() 走 Web Worker(mp3worker, helix mp3) 解码 mp3;
//   _986() 走 PCMData 库解码 wav; _993() 按扩展名分发。仅取声道 0
_878._978("_10/mp3worker/mp3worker.js");
_878._978("_10/pcmdata.js/binarystream.js");
_878._978("_10/pcmdata.js/pcmdata.js");
function _979(_939,_980,_981)
{
	if(!window.Worker)
	{
		_981("Mp3 decode error, worker not support.");
		return;
	}
	var _982=new Worker("_10/mp3worker/mp3worker.min.js");
	_982.onmessage=function(_983) 
	{
		let data = _983.data;
		let _984 = data["channelData"];
		if (!_984)
		{
			_981("Decode mp3 error.");
		}
		else
		{
			let _985 = _984[0];
			
			
			_980(_985, data.sampleRate);
		}
	};
	_982.onerror = function (_983)
	{
		alert(L("mp3解码错误，原因是：") + _983);
		_981("Decode mp3 error.");
	};
	_982.postMessage(_939);
}
function _986(_939,_980,_981)
{
	var _987=new Blob([_939])
	var _988=new FileReader();
	_988.onload=function(event)
	{
		var _989=event.target.result;
		_989=_989.substr("data:application/octet-stream;base64,".length);
		let _990=PCMData["decode"](atob(_989));
		let _991;
		_991=_990.data
		if(_990.channelCount==2)
		{
			let _992;
			_992=new Float32Array(_991.length/2);
			for(let _866=0;_866<_992.length;_866++)
				_992[_866]=_991[_866*2];
			_980(_992, _990.sampleRate);
		}
		else
			_980(_991, _990.sampleRate);
	};
	_988.readAsDataURL(_987);
}
function _993(name,_939,_980,_981)
{
	let _994=name.substring(name.lastIndexOf(".")+1);
	_994=_994.toLowerCase();
	if(_994=="mp3")
		_979(_939,_980,_981);
	else if(_994=="wav")
		_986(_939,_980,_981);
	else
		_981(L("只支持MP3和WAV格式！"));
}

