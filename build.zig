const std = @import("std");

const c_flags = [_][]const u8{
    "-std=c23",
    "-fblocks",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wshadow",
    "-Wconversion",
    "-Wdouble-promotion",
    "-Wformat=2",
    "-Wundef",
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const sigma_malloc = b.dependency("sigma_malloc", .{});
    const horny_mode = b.option(bool, "horny-mode", "Enable allocator HORNY_MODE") orelse (optimize == .Debug);
    const no_leak_reward = b.option(bool, "no-leak-reward", "Enable allocator NO_LEAK_REWARD") orelse (optimize == .Debug);
    const use_debug_alloc = b.option(bool, "use-debug-alloc", "Enable allocator debug metadata") orelse (optimize == .Debug);
    const use_malloc_backend = b.option(bool, "malloc-backend", "Use malloc/free as the allocator backing source") orelse false;

    const exe = b.addExecutable(.{
        .name = "sigma_rt",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    exe.use_llvm = true;
    exe.root_module.link_libc = true;
    exe.root_module.addIncludePath(b.path("include"));
    exe.root_module.addIncludePath(sigma_malloc.path("include"));
    exe.root_module.addCSourceFiles(.{
        .files = &.{ "main.c", "src/args.c", "src/rt.c" },
        .flags = &c_flags,
    });
    exe.root_module.addCSourceFiles(.{
        .root = sigma_malloc.path(""),
        .files = &.{
            "src/alloc.c",
            "src/arena.c",
            "src/arena_allocator.c",
            "src/buddy.c",
            "src/debug.c",
            "src/debug_free.c",
            "src/free.c",
            "src/large.c",
            "src/libc_wrappers.c",
            "src/memory_source_malloc.c",
            "src/memory_source_mmap.c",
            "src/sigma_malloc.c",
            "src/slab.c",
            "src/utils/bzero.c",
        },
        .flags = &c_flags,
    });
    if (horny_mode) exe.root_module.addCMacro("HORNY_MODE", "1");
    if (no_leak_reward) exe.root_module.addCMacro("NO_LEAK_REWARD", "1");
    if (use_debug_alloc) exe.root_module.addCMacro("USE_DEBUG_ALLOC", "1");
    if (use_malloc_backend) exe.root_module.addCMacro("SIGMA_MALLOC_BACKEND", "1");
    b.installArtifact(exe);

    const run = b.addRunArtifact(exe);
    run.step.dependOn(b.getInstallStep());
    if (b.args) |args| run.addArgs(args);
    b.step("run", "Run the sigma_rt example").dependOn(&run.step);

    _ = b.step("test", "Run sigma_rt tests when added");
}
