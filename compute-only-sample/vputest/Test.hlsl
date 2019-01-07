struct BufType
{
    int i;
    float f;
};

RWStructuredBuffer<BufType> Buffer0 : register(u0);
RWStructuredBuffer<BufType> Buffer1 : register(u1);
RWStructuredBuffer<BufType> BufferOut : register(u2);

// NOTE: we need to use 4 (intead of 1) for threads in the group to avoid WARP bug
[numthreads(4, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    BufferOut[DTid.x].i = Buffer0[DTid.x].i + Buffer1[DTid.x].i;
    BufferOut[DTid.x].f = Buffer0[DTid.x].f + Buffer1[DTid.x].f;
}
