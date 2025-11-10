while (running):
    begin_frame()
    if button("Run Simulation"):
        run_sim()
    slider("Iterations", &n, 1, 100)
    draw_scene()
    end_frame()
