#include <ntddk.h>

#define PCKILLER_DEVICE_TYPE 0x8000
#define IOCTL_SHUTDOWN CTL_CODE(PCKILLER_DEVICE_TYPE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CANCEL CTL_CODE(PCKILLER_DEVICE_TYPE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REMAINING CTL_CODE(PCKILLER_DEVICE_TYPE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)



typedef enum _SHUTDOWN_ACTION {
    ShutdownNoReboot,
    ShutdownReboot,
    ShutdownPowerOff
} SHUTDOWN_ACTION;


typedef NTSTATUS(*NT_SHUTDOWN_SYSTEM)(SHUTDOWN_ACTION Action);
NT_SHUTDOWN_SYSTEM NtShutdownSystem = NULL;


static LARGE_INTEGER ShutdownTime;
static BOOLEAN ShutdownScheduled = FALSE;
static KTIMER Timer;
static KDPC TimerDpc;



VOID MyShutdownSystem(_In_ PDEVICE_OBJECT DeviceObject, _In_opt_ PVOID Context){
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Context);

    if (NtShutdownSystem) {
        NTSTATUS status = NtShutdownSystem(ShutdownPowerOff);
        if (!NT_SUCCESS(status)) {
            KdPrint(("PCKiller: NtShutdownSystem failed with status: 0x%08X\n", status));
        }
    }

    else {
        KdPrint(("PCKiller: NtShutdownSystem not found.\n"));
    }
}


VOID ShutdownDpcRoutine(_In_ struct _KDPC* Dpc, _In_opt_ PVOID DeferredContext, _In_opt_ PVOID SystemArgument1, _In_opt_ PVOID SystemArgument2){
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    KdPrint(("PCKiller: ShutdownDpcRoutine called. Shutting down the system.\n"));

    PDEVICE_OBJECT DeviceObject = (PDEVICE_OBJECT)DeferredContext;
    if (DeviceObject != NULL) {
        PIO_WORKITEM pWorkItem = IoAllocateWorkItem(DeviceObject);
        if (pWorkItem != NULL) {
            IoQueueWorkItem(pWorkItem, MyShutdownSystem, DelayedWorkQueue, NULL);
        }

        else {
            KdPrint(("PCKiller: Failed to allocate work item.\n"));
        }
    }

    else {
        KdPrint(("PCKiller: DeviceObject is NULL.\n"));
    }
}


NTSTATUS DriverIoControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG controlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG inputBufferLength = stack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG outputBufferLength = stack->Parameters.DeviceIoControl.OutputBufferLength;
    NTSTATUS status = STATUS_SUCCESS;

    switch (controlCode) {
        case IOCTL_SHUTDOWN:
            if (inputBufferLength == sizeof(ULONG)) {
                ULONG n = *(ULONG*)buffer;
                LARGE_INTEGER currentTime;
                KeQuerySystemTime(&currentTime);
                ShutdownTime.QuadPart = currentTime.QuadPart + (LONGLONG)n * 10000000LL; // Convert seconds to 100-nanosecond intervals
                KeSetTimer(&Timer, ShutdownTime, &TimerDpc);
                ShutdownScheduled = TRUE;
                KdPrint(("PCKiller: Shutdown scheduled in %lu seconds.\n", n));
            }

            else {
                status = STATUS_INVALID_PARAMETER;
                KdPrint(("PCKiller: Invalid parameter for IOCTL_SHUTDOWN.\n"));
            }

            break;

        case IOCTL_CANCEL:
            KeCancelTimer(&Timer);
            ShutdownScheduled = FALSE;
            KdPrint(("PCKiller: Shutdown canceled.\n"));
            break;

        case IOCTL_REMAINING:
            if (outputBufferLength == sizeof(LARGE_INTEGER)) {
                if (ShutdownScheduled) {
                    LARGE_INTEGER currentTime;
                    KeQuerySystemTime(&currentTime);
                    LARGE_INTEGER remainingTime;
                    remainingTime.QuadPart = ShutdownTime.QuadPart - currentTime.QuadPart;
                    if (remainingTime.QuadPart < 0) {
                        remainingTime.QuadPart = 0;
                    }
                    RtlCopyMemory(buffer, &remainingTime, sizeof(LARGE_INTEGER));
                }

                else {
                    LARGE_INTEGER noShutdown;
                    noShutdown.QuadPart = -1;
                    RtlCopyMemory(buffer, &noShutdown, sizeof(LARGE_INTEGER));
                    KdPrint(("PCKiller: No shutdown scheduled.\n"));
                }
            }

            else {
                status = STATUS_INVALID_PARAMETER;
                KdPrint(("PCKiller: Invalid parameter for IOCTL_REMAINING.\n"));
            }

            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            KdPrint(("PCKiller: Invalid device request.\n"));
            break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = (status == STATUS_SUCCESS) ? outputBufferLength : 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}


NTSTATUS DriverCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


VOID UnloadDriver(PDRIVER_OBJECT DriverObject){
    KdPrint(("PCKiller: Unloading driver.\n"));
    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\PCKiller");
    IoDeleteSymbolicLink(&symbolicLink);
    IoDeleteDevice(DriverObject->DeviceObject);
}


extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath){
    UNREFERENCED_PARAMETER(RegistryPath);

    DriverObject->DriverUnload = UnloadDriver;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverIoControl;

    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\PCKiller");
    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\PCKiller");

    PDEVICE_OBJECT DeviceObject = NULL;
    NTSTATUS status = IoCreateDevice(
        DriverObject,
        0,
        &deviceName,
        PCKILLER_DEVICE_TYPE,
        0,
        FALSE,
        &DeviceObject
    );

    if (!NT_SUCCESS(status)) {
        KdPrint(("PCKiller: IoCreateDevice failed with status: 0x%08X\n", status));
        return status;
    }

    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
    if (!NT_SUCCESS(status)) {
        KdPrint(("PCKiller: IoCreateSymbolicLink failed with status: 0x%08X\n", status));
        IoDeleteDevice(DeviceObject);
        return status;
    }

    KeInitializeTimer(&Timer);
    KeInitializeDpc(&TimerDpc, ShutdownDpcRoutine, DeviceObject);

    // Resolve NtShutdownSystem dynamically
    UNICODE_STRING routineName = RTL_CONSTANT_STRING(L"NtShutdownSystem");
    NtShutdownSystem = (NT_SHUTDOWN_SYSTEM)MmGetSystemRoutineAddress(&routineName);
    if (!NtShutdownSystem) {
        KdPrint(("PCKiller: Failed to resolve NtShutdownSystem.\n"));
        IoDeleteSymbolicLink(&symbolicLink);
        IoDeleteDevice(DeviceObject);
        return STATUS_UNSUCCESSFUL;
    }

    KdPrint(("PCKiller: Driver loaded successfully.\n"));
    return STATUS_SUCCESS;
}