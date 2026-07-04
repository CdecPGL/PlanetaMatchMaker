using System;

namespace Open.Nat
{
	/// <summary>
	/// Protocol that should be used for searching a NAT device. 
	/// </summary>
	[Flags]
	public enum PortMapper
	{
		/// <summary>
		/// Use only Universal Plug and Play
		/// </summary>
		Upnp = 1
	}
}
